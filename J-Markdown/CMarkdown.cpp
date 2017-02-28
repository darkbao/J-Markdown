#include "CMarkdown.h"
#include <cstdlib>
#include <regex>
#include <unordered_map>
#include <ctype.h>

using std::wstring;
typedef wstring::const_iterator const_iter;
using std::unordered_map;
using std::wregex;
using std::wsregex_iterator;
using std::regex_replace;


CMarkdown::CMarkdown(const CHtmlRender & rndr)
:render(rndr)
{

}

CMarkdown::~CMarkdown(){

}

wstring CMarkdown::tranform(const wstring &input)
{	
	wstring res;
	if (input.empty())
		return res;
	unordered_map<wstring, wstring> ref_map;
	wstring processed_input = preProcessing(input,ref_map);
	const_iter beg = processed_input.begin();
	const_iter end = processed_input.end();
	size_t count = 0;

	while (beg != end){
		
		if (is_atxheader(beg, end))
			procAtxHeader(res, beg, end, ref_map);

		else if (is_html_label(beg, end))
			procHtmlLabel(res, beg, end);

		else if ((count = is_parting_line(beg, end)) != 0)
		{
			render.renderSeparator(res);
			beg += count;
		}
		
		else if (is_blockquote(beg, end))
			procBlockquote(res, beg, end);

		else if ((count = is_olist(beg, end)))
			procOlist(res, beg, end, count,ref_map);

		else if ((count = is_ulist(beg, end)) != 0)	
			procUlist(res, beg, end, count,ref_map);
		
		else if (is_code(beg, end))
			procCode(res, beg, end);

		else if ((count = empty_line(beg, end)) != 0)
			beg += count;

		else
			procParagraph(res, beg, end,ref_map);
	}
	return res;
}

size_t CMarkdown::empty_line(const_iter beg, const_iter end){
	size_t count = 0;
	const_iter line_end = beg;
	while (beg != end && *beg == L' '){
		++count;
		++beg;
	}
	if (beg != end && *beg == L'\n')
		return count + 1;
	else
		return 0;
}

wstring CMarkdown::preProcessing(const wstring &input,unordered_map<wstring, wstring> &ref_map){


	wstring out;
	/*tranform special char in HTML, and expand every tab char into four spaces*/
	for (const_iter it = input.begin(); it!=input.end() ; ++it){
		if (*it == L'\t')
			out += L"    ";
		else
			out += *it;
	}
	/*push back a line break in the end of input*/
	if (out.size() > 0 && *(out.end() - 1) != L'\n')
		out.push_back(L'\n');
	/*find all references of images and web links,store the result in an unorder_map*/
	wstring ref_pattern(L"^ {0,3}\\[(.+?)\\]: *(.+?) *$");
	wregex wr(ref_pattern);
	
	for (wsregex_iterator i(out.begin(), out.end(), wr), endit; i != endit; ++i)
			ref_map[stringToUpper(i->str(1))] = i->str(2);

	/*delete all reference line*/
	wstring delete_fmt = L"";
	wstring output = regex_replace(out, wr, delete_fmt);
	/*wcout << "References: \n";
	for (auto i = ref_map.begin(); i != ref_map.end(); ++i)
		wcout << i->first << " to " << i->second << endl;
	wcout << endl;*/
	return output;
}

void CMarkdown::procAtxHeader(wstring & res, const_iter & beg, const_iter end, unordered_map<wstring, wstring> &ref_map){
	size_t blank_count = 0;
	while (beg != end && *beg == L' ' && blank_count < 3){
		++blank_count;
		++beg;
	}
	beg += blank_count;
	const_iter end_line = beg;
	while (end_line != end && *end_line != L'\n')
		++end_line;
	size_t count = 0;
	while (beg != end && *beg == L'#'){
		++count; ++beg;
	}
	while (beg != end && *beg == L' '){
		++beg;
	}
	const_iter end_text = end_line;
	if (end_text != beg)
		--end_text;
	while (end_text != beg && *end_text == L'#')
		--end_text;
	while (end_text != beg && *end_text == L' ') 
		--end_text;
	render.renderHeader(res, procInline(beg, end_text + 1, ref_map), count);
	beg = end_line + 1;
}

void CMarkdown::procHtmlLabel(wstring &res, const_iter &beg, const_iter end)
{
	auto label_beg = beg;
	while (beg!=end)
	{
		while ((++beg) != end && *beg != '\n')
			;
		size_t count = 0;
		if (beg != end && (++beg) != end && (count = empty_line(beg, end)) != 0)
		{
			beg += count;
			break;
		}
	}
	res += wstring(label_beg, beg);
}

void CMarkdown::procBlockquote(wstring & res, const_iter & beg, const_iter end){
	const_iter line_end = beg;
	wstring tmp;
	bool last_line_empty = true;
	while (beg != end){
		while (*line_end != L'\n')
			++line_end;
		++line_end;

		if (is_empty(beg, line_end)){
			tmp += L'\n';
			beg = line_end;
			last_line_empty = true;
			continue;
		}
		
		if (is_blockquote(beg, line_end)){
			while (beg != end && *beg == L' ')
				++beg;
			if (*(beg + 1) == L' ')
				tmp += wstring(beg + 2, line_end);
			else
				tmp += wstring(beg + 1, line_end);
			beg = line_end;
			last_line_empty = false;
		}
		else if (last_line_empty){
			break;
		}
		else{
			tmp += wstring(beg, line_end);
			beg = line_end;
			last_line_empty = false;
		}
	}
	render.renderBlockquote(res, tranform(tmp));
}

void CMarkdown::procOlist(wstring & res, const_iter & beg, const_iter end, size_t first_list_item_prefix, 
							unordered_map<wstring, wstring> &ref_map)
{
	render.renderOlist(res, CHtmlRender::front);
	const_iter list_beg = beg;
	const_iter line_end = beg;
	bool has_empty_line = false, 
		has_nonempty_line = false,
		process_as_para = false;
	size_t actual_first_item_prefix = actual_prefix_olist(beg, end);

	while (beg != end){
		while (*line_end != L'\n')
			++line_end;
		++line_end;

		if (line_end == end)//reach the end of input
		{
			if (!is_empty(beg, line_end) && has_empty_line)
				has_nonempty_line = true;
			procListItem(res, list_beg, end, process_as_para || has_nonempty_line,
				first_list_item_prefix, ref_map);
			beg = end;
			break;
		}

		/*obtain a new line*/
		const_iter new_Line_end = line_end;
		while (*new_Line_end != L'\n')
			++new_Line_end;
		++new_Line_end;

		size_t next_prefix = actual_prefix_olist(line_end, new_Line_end);

		if (is_empty(beg, line_end))//current line is empty line
		{
			has_empty_line = true;
			if (*(line_end) != L' ' && *(line_end) != L'\n' &&
				next_prefix == 0)//reach the end of this list block
			{
				procListItem(res, list_beg, line_end, process_as_para || has_nonempty_line,
					first_list_item_prefix, ref_map);
				beg = line_end;
				break;
			}
			else if (next_prefix>0 && next_prefix <= actual_first_item_prefix)//reach the end of sub list item
			{
				process_as_para = true;
				procListItem(res, list_beg, line_end, process_as_para, first_list_item_prefix, ref_map);
				list_beg = beg = line_end;
			}
			else
			{
				beg = line_end;
			}
		}
		else
		{
			if (next_prefix>0 && next_prefix <= actual_first_item_prefix)//reach the end of sub list item
			{
				process_as_para = ((has_empty_line == true) ? true : false);
				procListItem(res, list_beg, line_end, process_as_para, first_list_item_prefix, ref_map);
				list_beg = beg = line_end;
			}
			else{
				beg = line_end;
			}
		}

	}
	render.renderOlist(res, CHtmlRender::rear);
}

void CMarkdown::procUlist(wstring & res, const_iter & beg, const_iter end, size_t first_list_item_prefix,
							unordered_map<wstring, wstring> &ref_map){
	render.renderUlist(res, CHtmlRender::front);
	const_iter list_beg = beg;
	const_iter line_end = beg;
	bool has_empty_line = false,
		has_nonempty_line = false, 
		process_as_para = false;

	while (beg != end){
		while (*line_end != L'\n')
			++line_end;
		++line_end;

		if (line_end == end)//reach the end of input
		{
			if (!is_empty(beg, line_end) && has_empty_line)
				has_nonempty_line = true;
			procListItem(res, list_beg, end, process_as_para || has_nonempty_line,
				first_list_item_prefix, ref_map);
			beg = end;
			break;
		}

		/*obtain a new line*/
		const_iter new_Line_end = line_end;
		while (*new_Line_end != L'\n')
			++new_Line_end;
		++new_Line_end;
		size_t next_prefix = is_ulist(line_end, new_Line_end);

		if (is_empty(beg, line_end))//current line is empty line
		{		
			has_empty_line = true;
			if ( *(line_end) != L' ' && *(line_end) != L'\n' && 
				next_prefix==0)//reach the end of this list block
			{
				procListItem(res, list_beg, line_end, process_as_para || has_nonempty_line, 
					first_list_item_prefix, ref_map);
				beg = line_end;
				break;
			}
			else if (next_prefix>0 && next_prefix <= first_list_item_prefix)//reach the end of sub list item
			{
				process_as_para = true;
				procListItem(res, list_beg, line_end, process_as_para, first_list_item_prefix, ref_map);
				list_beg = beg = line_end;
			}
			else
			{
				beg = line_end;
			}
		}
		else
		{
			has_nonempty_line = ((has_empty_line == true) ? true : false);
			if (next_prefix>0 && next_prefix <= first_list_item_prefix)//reach the end of sub list item
			{
				process_as_para = ((has_empty_line == true) ? true : false);
				procListItem(res, list_beg, line_end, process_as_para, first_list_item_prefix, ref_map);
				list_beg = beg = line_end;
			}
			else
			{
				beg = line_end;
			}
		}

	}
	render.renderUlist(res, CHtmlRender::rear);
}

void CMarkdown::procListItem(wstring & res, const_iter beg, const_iter end, bool has_empty_line, size_t prefix_len, unordered_map<wstring, wstring> &ref_map){
	render.renderListItem(res, CHtmlRender::front);
	beg += prefix_len;
	const_iter line_end = beg;
	wstring removed_prefix_text;
	while (*line_end != L'\n')
		++line_end;
	++line_end;
	removed_prefix_text += wstring(beg, line_end);

	while (line_end != end){
		size_t count = 0;
		while (*line_end != L'\n' && *line_end == L' ' && count < 4){
			++count; ++line_end;
		}
		const_iter text_beg = line_end;
		while (*line_end != L'\n'){
			++line_end;
		}
		++line_end;
		removed_prefix_text += wstring(text_beg, line_end);
	}
	if (has_empty_line)
	{
		res += tranform(removed_prefix_text);
	}
	else
	{
		const_iter list_content_end = removed_prefix_text.begin();
		while (*list_content_end != L'\n')
			++list_content_end;
		++list_content_end;
		const_iter line_beg = list_content_end;

		while (list_content_end != removed_prefix_text.end()){
			while (*list_content_end != L'\n')
				++list_content_end;
			++list_content_end;
			if (is_ulist(line_beg, list_content_end) || is_olist(line_beg, list_content_end))
				break;
			else
				line_beg = list_content_end;
		}
		res += procInline(removed_prefix_text.begin(), line_beg, ref_map) +
			tranform(wstring(line_beg, removed_prefix_text.end()));
	}
	render.renderListItem(res, CHtmlRender::rear);
}

void CMarkdown::procCode(wstring & res, const_iter & beg, const_iter end){
	wstring tmp;
	const_iter line_end = beg;
	while (beg!=end)
	{
		while (*line_end != L'\n')
			++line_end;
		++line_end;
		if (is_code(beg, line_end)){
			beg += 4;
			for (; beg != line_end; ++beg){
				render.tranSpecialchar(*beg, tmp);
			}
		}
		else
			break;
	}
	render.renderCode(res, tmp);
}

void CMarkdown::procParagraph(wstring & res, const_iter & beg, const_iter end, std::unordered_map<wstring, wstring> &ref_map){
	const_iter paragraph_beg = beg;
	const_iter line_end = beg; 
	while (beg != end){
		while (*line_end != L'\n')
			++line_end;
		++line_end;
		size_t level = 0;
		if (is_empty(beg, line_end)){			
			render.renderParagraph(res, procInline(paragraph_beg, beg, ref_map));
			beg = line_end;
			return;
		}
		else if ((level = is_setextheader(beg,line_end)) != 0){
			/*the setext type head is the first line of the paragraph*/
			if (beg == paragraph_beg){
				beg = line_end;
				return;
			}
			/*backtrack to the last line*/
			else
			{
				auto setext_head_end = beg;
				beg -= 2;
				while (beg != paragraph_beg && *beg != L'\n')
					--beg;
				if (beg == paragraph_beg)
					render.renderSetextHeader(res, procInline(paragraph_beg, setext_head_end, ref_map), level);
				else
					render.renderSetextHeader(res, procInline(beg + 1, setext_head_end, ref_map), level);
				beg = line_end;
				return;
			}
		}
		else if (is_atxheader(beg, line_end) || is_blockquote(beg, line_end) || is_parting_line(beg,line_end)){
			render.renderParagraph(res, procInline(paragraph_beg, beg, ref_map));
			return;
		}
		else
			beg = line_end;
	}
	render.renderParagraph(res, procInline(paragraph_beg, line_end, ref_map));
}

wstring CMarkdown::procInline(const_iter beg, const_iter end, std::unordered_map<wstring, wstring> &ref_map){
	wstring tmp;
	while (beg != end){
		switch (*beg)
		{
			/*handle escape characters*/
		case L'\\':{
					   ++beg;
					   if (*(beg) == L'\\' || *(beg) == L'`' || *(beg) == L'*' || *(beg) == L'_'
						   || *(beg) == L'{' || *(beg) == L'}' || *(beg) == L'[' || *(beg) == L']'
						   || *(beg) == L'(' || *(beg) == L')' || *(beg) == L'#' || *(beg) == L'+'
						   || *(beg) == L'-' || *(beg) == L'.' || *(beg) == L'!'){
						   tmp += *beg;
					   }
					   else
						   tmp += (L'\\' + *beg);
					   ++beg;
		}
			break;
			/*deal with inline code*/
		case L'`':{
					  size_t count = 1;
					  while (*(++beg) == L'`')
						  ++count;
					  wstring left = wstring(beg + 1, end);
					  auto beg_copy = beg;
					  if (!left.empty()){
						  for (size_t i = count; i > 0; i--){
							  auto pos = left.find(wstring(i, L'`'));
							  if (pos != wstring::npos){
								  tmp += wstring(count - i, L'`');
								  render.renderInlineCode(tmp, render.tranSpecialchar(wstring(beg, beg + 1 + pos))); 
								  beg += pos + 1 + i;
								  break;
							  }
						  }
					  }
					  if (beg == beg_copy)
						  tmp += wstring(count, L'`');
		}
			break;	
			/*handle emphasis*/
		case L'*':{
					  size_t count = 1;
					  while (*(++beg) == L'*')
						  ++count;
					  wstring left = wstring(beg + 1, end);
					  if (count == 1){
						  auto pos = left.find(L'*');
						  if (pos != wstring::npos){
							  render.renderEmphasis(tmp, procInline(beg, beg + 1 + pos, ref_map),CHtmlRender::em_t);
							  beg += pos + 2;
						  }
						  else
							  tmp += L'*';						
					  }
					  else if (count == 2)
					  {
						  auto pos = left.find(L"**");
						  if (pos != wstring::npos){
							  render.renderEmphasis(tmp, procInline(beg, beg + 1 + pos, ref_map),CHtmlRender::str_t);
							  beg += pos + 3;
						  }
						  else
							  tmp += L"**";
					  }
					  else
					  {
						  auto beg_copy = beg;
						  for (size_t i = 3; i > 0; i--)
						  {
							  auto pos = left.find(wstring(i, L'*'));
							  if (pos != wstring::npos){
								  tmp += wstring(count - i, L'*');
								  render.renderEmphasis(tmp, procInline(beg, beg + 1 + pos, ref_map), CHtmlRender::emphasis_type(i - 1));
								  beg += pos + 1 + i;
								  break;
							  }
						  }
						  if (beg == beg_copy)
							  tmp += wstring(count, L'*');
					  }
		}
			break;
		case L'_':{
					  size_t count = 1;
					  while (*(++beg) == L'_')
						  ++count;
					  wstring left = wstring(beg + 1, end);
					  if (count == 1){
						  auto pos = left.find(L'_');
						  if (pos != wstring::npos){
							  render.renderEmphasis(tmp, procInline(beg, beg + 1 + pos, ref_map), CHtmlRender::em_t);
							  beg += pos + 2;
						  }
						  else
							  tmp += L'_';
					  }
					  else if (count == 2)
					  {
						  auto pos = left.find(L"__");
						  if (pos != wstring::npos){
							  render.renderEmphasis(tmp, procInline(beg, beg + 1 + pos, ref_map), CHtmlRender::str_t);
							  beg += pos + 3;
						  }
						  else
							  tmp += L"__";
					  }
					  else
					  {
						  auto beg_copy = beg;
						  for (size_t i = 3; i > 0; i--)
						  {
							  auto pos = left.find(wstring(i, L'_'));
							  if (pos != wstring::npos){
								  tmp += wstring(count - i, L'_');
								  render.renderEmphasis(tmp, procInline(beg, beg + 1 + pos, ref_map), CHtmlRender::emphasis_type(i - 1));
								  beg += pos + 1 + i;
								  break;
							  }
						  }
						  if (beg == beg_copy)
							  tmp += wstring(count, L'_');
					  }
		}
			break;
			/*handle image*/
		case L'!':
		{
					bool has_rendered = false;
					if (*(++beg) != L'[')
					{
						has_rendered = true;
						tmp += L'!';
					}
					else
					{
						auto first_item_beg = beg + 1;
						wstring first_item;
						while (++beg != end)
						{
							if (*beg == L']' && *(beg-1) != L'\\')
							{
								first_item = wstring(first_item_beg, beg);
								break;
							}
							if (*beg == L'[' && *(beg - 1) != L'\\')
								break;
						}
						if (beg != end && *beg == L']')
						{
							while (++beg != end && (*beg == L' ' || *beg == L'\n'))
								;
							/*inline style image link*/
							if (beg != end && *beg == L'(')
							{
								auto second_item_beg = beg + 1;
								wstring second_item;
								while (++beg != end)
								{
									if (*beg == L')' && *(beg - 1) != L'\\')
									{
										second_item = wstring(second_item_beg, beg);
										break;
									}
									if (*beg == L'('  && *(beg - 1) != L'\\')
										break;
								}
								if (beg != end && *beg == L')')
								{
									render.renderImage(tmp, procInline(first_item, ref_map), procInline(second_item, ref_map));
									has_rendered = true;
									++beg;
								}
							}
							/*reference style image link*/
							if (beg != end && *beg == L'[')
							{
								auto second_item_beg = beg + 1;
								wstring second_item;
								while (++beg != end)
								{
									if (*beg == L']' && *(beg - 1) != L'\\')
									{
										second_item = wstring(second_item_beg, beg);
										break;
									}
									if (*beg == L'[' && *(beg - 1) != L'\\')
										break;
								}
								if (beg != end && *beg == L']')
								{
									wstring id = stringToUpper(second_item.empty() ? first_item : second_item);
									if (ref_map.count(id))
									{
										render.renderImage(tmp, procInline(first_item, ref_map), procInline(ref_map[id], ref_map));
										has_rendered = true;
										++beg;
									}
								}
							}
						}
						if (!has_rendered)
						{
							tmp += L"![";
							beg = first_item_beg;
						}
					}
		}
			break;
			/*handle link*/
		case L'[':
		{
					bool has_rendered = false;				 					 
					auto first_item_beg = beg + 1;
					wstring first_item;
					while (++beg != end)
					{
						if (*beg == L']' && *(beg - 1) != L'\\')
						{
							first_item = wstring(first_item_beg, beg);
							break;
						}
						if (*beg == L'[' && *(beg - 1) != L'\\')
							break;
					}
					if (beg != end && *beg == L']')
					{
						while (++beg != end && (*beg == L' ' || *beg == L'\n'))
							;
						/*inline style link*/
						if (beg != end && *beg == L'(')
						{
							auto second_item_beg = beg + 1;
							wstring second_item;
							while (++beg != end)
							{
								if (*beg == L')' && *(beg - 1) != L'\\')
								{
									second_item = wstring(second_item_beg, beg);
									break;
								}
								if (*beg == L'('  && *(beg - 1) != L'\\')
									break;
							}
							if (beg != end && *beg == L')')
							{
								render.renderLink(tmp, procInline(first_item, ref_map), procInline(second_item, ref_map));
								has_rendered = true;
								++beg;
							}
						}
						/*reference style link*/
						if (beg != end && *beg == L'[')
						{
							auto second_item_beg = beg + 1;
							wstring second_item;
							while (++beg != end)
							{
								if (*beg == L']' && *(beg - 1) != L'\\')
								{
									second_item = wstring(second_item_beg, beg);
									break;
								}
								if (*beg == L'[' && *(beg - 1) != L'\\')
									break;
							}
							if (beg != end && *beg == L']')
							{
								wstring id = stringToUpper(second_item.empty() ? first_item : second_item);
								if (ref_map.count(id))
								{
									render.renderLink(tmp, procInline(first_item, ref_map), procInline(ref_map[id], ref_map));
									has_rendered = true;
									++beg;
								}
							}
						}
					}
					if (!has_rendered)
					{
						tmp += L'[';
						beg = first_item_beg;
					}
		}
			break;
			/*handle inline html syntax*/
		case L'<':
		{
					auto beg_copy = beg;
					if ((++beg) != end && (isalpha(*beg) || *beg==L'/'))
					{
						while ((++beg) != end && *beg != L'\n' && *beg != L'>' && *beg != L'<')
							;
						if (beg != end && *beg == L'>')
						{
							tmp += wstring(beg_copy, ++beg);
							break;
						}
					}
					render.tranSpecialchar(*beg_copy, tmp);
					beg = beg_copy + 1;
		}
			break;
		case L'&':
		{
					auto beg_copy = beg;
					while ((++beg) != end && *beg != L'\n' && *beg != L';' && *beg != L'&')
						;
					if (beg != end && *beg == L';')
					{
						tmp += wstring(beg_copy, ++beg);
						break;
					}
					render.tranSpecialchar(*beg_copy, tmp);
					beg = beg_copy + 1;
		}
			break;
			/*check if there is a compulsive line break insertion*/
		case L'\n':
			if (tmp.size() >= 2 && *(tmp.cend() - 1) == L' ' && *(tmp.cend() - 2) == L' ')
				render.renderLineBreak(tmp);
			tmp += *beg;
			++beg;
			break;
		default:			
			tmp += *beg;
			++beg;
			break;
		}
	}
	return tmp;
}

wstring CMarkdown::procInline(const wstring &input, std::unordered_map<wstring, wstring> &ref_map){
	return procInline(input.cbegin(), input.cend(), ref_map);
}

bool CMarkdown::is_atxheader(const_iter beg,const_iter end)
{
	size_t blank_count = 0;
	while (beg != end && *beg == L' ' && blank_count < 3)
	{
		++blank_count;
		++beg;
	}
	if (*beg == L'#')
		return true;
	else
		return false;
}

size_t CMarkdown::is_setextheader(const_iter beg, const_iter end){
	if (beg == end)
		return 0;
	wchar_t token = *beg;
	size_t level = 0;
	if (token == L'=')
		level = 1;
	else if (token == L'-')
		level = 2;
	else
		return 0;
	while (++beg != end && *beg != L'\n'){
		if (*beg != token)
			return 0;
	}
	return level;
}

/*区块符号‘>’后面允许有一个空格，也可以没有，此函数计算区块前缀的字符数，包括空格*/
size_t CMarkdown::is_blockquote(const_iter beg, const_iter end){
	size_t blank_count = 0, count = 0;
	while (beg != end && *beg == L' ' && blank_count < 3){
		++blank_count;
		++beg;
	}
	while (beg != end && *beg == L'>') {
		++count;
		++beg;
	}
	if (count == 0)
		return 0;
	if (beg != end && *beg == L' ')
		return blank_count + count + 1;
	return blank_count + count;
}

size_t CMarkdown::is_olist(const_iter beg, const_iter end){
	const_iter cur = beg;
	while (cur != end && *cur == L' ')
		++cur;
	if (cur - beg>3 || cur == end || (cur != end && !iswdigit(*cur)))
		return 0;
	size_t digit_count = 0;
	while (cur != end && iswdigit(*cur)){
		++cur; ++digit_count;
	}
	if (digit_count == 0)
		return 0;
	else
	{
		if (end - cur < 2 || (end - cur >= 2 && wstring(cur, cur + 2) != L". "))
			return 0;
		else
			return cur - beg + 2;
	}
}
size_t  CMarkdown::actual_prefix_olist(const_iter beg, const_iter end)
{
	const_iter cur = beg;
	while (cur != end && *cur == L' ')
		++cur;
	size_t prefix = cur - beg;
	if (cur - beg>3 || cur == end || (cur != end && !iswdigit(*cur)))
		return 0;
	size_t digit_count = 0;
	while (cur != end && iswdigit(*cur)){
		++cur; ++digit_count;
	}
	if (digit_count == 0)
		return 0;
	else
	{
		if (end - cur < 2 || (end - cur >= 2 && wstring(cur, cur + 2) != L". "))
			return 0;
		else
			return prefix + 2;
	}
}
size_t CMarkdown::is_ulist(const_iter beg, const_iter end){
	size_t count = 0;
	while (beg != end && *beg == L' ' && count < 3){
		++count;
		++beg;
	}
	if (beg != end && (*beg == L'*' || *beg == L'-' || *beg == L'+')){
		if (beg + 1 != end && *(beg + 1) == L' ')
			return count+2;
	}
	return 0;
}

bool CMarkdown::is_code(const_iter beg, const_iter end){
	if (end - beg >= 4 && wstring(beg, beg + 4) == L"    ")
		return true;
	else
		return false;
}

size_t CMarkdown::is_empty(const_iter beg, const_iter end){
	size_t count = end - beg;
	while (beg != end){
		if (*beg != L' ' && *beg != L'\n')
			return 0;
		++beg;
	}
	return count;
}

//wstring CMarkdown::procInline(const_iter beg, const_iter end,unordered_map<wstring, wstring> &ref_map){
//	/*save the code text to wsregex_iterator it, and replace it with wstring `C`*/
//	using std::wregex;
//	using std::regex_constants::format_first_only;
//	using std::vector;
//	
//	wstring code(L"`([^`].+?)`|``(.+?)``");
//	wstring fmt_code(L"`C`");
//	wregex wr(code);
//	vector<wstring> code_vtor;
//	for (wsregex_iterator it(beg, end, wr), endit; it != endit; ++it){
//		code_vtor.push_back(L"<code>" + render.tranSpecialchar(it->str(1) + it->str(2)) + L"</code>");
//	}
//	wstring res = regex_replace(wstring(beg, end), wr, fmt_code);
//	/*process other Segment element*/
//	wstring strong(L"(\\*\\*)([^\\*].+?)(\\*\\*)");//the shortest match
//	wstring	emphasis(L"(\\*)([^\\*].+?)(\\*)");
//	wstring link(L"\\[(.+?)\\]\\((.+?) *\"(.+?)\"\\)|\\[(.+?)\\]\\((.+?) *\\)");
//	wstring img(L"!\\[(.+?)\\]\\((.+?) *\"(.+?)\"\\)|!\\[(.+?)\\]\\((.+?) *\\)");
//
//	wstring fmt_strong(L"<strong>$2</strong>");
//	wstring fmt_emphasis(L"<em>$2</em>");
//	wstring fmt_link(L"<a href = \"$2$5\" title = \"$3\">$1$4</a>");
//	wstring fmt_img(L"<img src = \"$2$5\" alt = \"$1$4\" title = \"$3\">");
//
//	wr = strong;
//	res = regex_replace(res, wr, fmt_strong);
//	wr = emphasis;
//	res = regex_replace(res, wr, fmt_emphasis);
//	wr = img;
//	res = regex_replace(res, wr, fmt_img);
//	wr = link;
//	res = regex_replace(res, wr, fmt_link);
//
//	wstring ref(L"\\[(.+?)\\] *\\[(.*?) *\\]|!\\[(.+?)\\] *\\[(.*?) *\\]");
//	wr = ref;
//	unordered_map<wstring, wstring> found_ref_map;
//	for (wsregex_iterator it(res.begin(), res.end(), wr), endit; it != endit; ++it){
//		if (it->str(0)[0] != L'!'){
//			wstring id(!it->str(2).empty() ? it->str(2) : it->str(1));
//			id = stringToUpper(id);
//			if (ref_map.count(id)){
//				wstring tmp(L"<a href = " + ref_map[id] + it->str(1) + L"</a>");
//				found_ref_map[it->str(0)] = tmp;
//				
//			}
//		}
//		else{
//			wstring id(!it->str(4).empty() ? it->str(4) : it->str(3));
//			id = stringToUpper(id);
//			if (ref_map.count(id)){
//				wstring tmp(L"<img src = " + ref_map[id] + L" alt = \"" + it->str(3) + L"\">");
//				found_ref_map[it->str(0)] = tmp;
//			}
//		}				
//	}
//	for (auto b = found_ref_map.begin(); b != found_ref_map.end(); ++b){
//		res.replace(res.find(b->first), b->first.size(), b->second);
//	}
//	/*recover the inline code part*/
//	size_t pos = 0;
//	auto i = code_vtor.begin();
//	while ((pos = res.find(fmt_code)) != wstring::npos && i != code_vtor.end()){
//		res.replace(pos, 3, *i);
//		++i;
//	}
//	return res;
//}

wstring CMarkdown::stringToUpper(const wstring &input){
	wstring re;
	for (auto i = input.begin(); i != input.end(); ++i)
	{
		re += towupper(*i);
	}
	return re;
}

size_t CMarkdown::is_parting_line(const_iter beg, const_iter end){
	const_iter it = beg;
	size_t blank_count = 0;
	while (it != end && *it == L' ' && blank_count<4){
		++it;
		++blank_count;
	}
	if (it == end || blank_count == 4)
		return 0;
	if (*it == L'*' || *it == L'-' || *it == L'_'){
		wchar_t token = *it;
		size_t count = 1;
		while (++it != end){
			if (*it == L'\n')
				break;
			if (*it == token)
				++count;
			else if (*it == L' ')
				continue;
			else
				return 0;
		}
		if (count < 3)
			return 0;
		else{
			if (it == end)
				return it - beg;
			else
				return it - beg + 1;
		}
	}
	else
		return 0;
}

/*assume the longest label's length of HTML is 20*/
bool CMarkdown::is_html_label(const_iter beg, const_iter end)
{
	if (*beg == L'<')
	{
		if ((++beg) != end && isalpha(*beg))
		{
			while ((++beg) != end && *beg != L'\n' && *beg != L'>'&& *beg != L'<')
				;
			if (beg != end && *beg == L'>')
				return true;
		}
	}
	return false;
}
