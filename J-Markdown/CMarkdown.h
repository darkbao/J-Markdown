#ifndef _CMARKDOWN_H
#define _CMARKDOWN_H

#include <string>
#include <unordered_map>
#include "CHtmlRender.h"

class CMarkdown
{
	typedef std::wstring wstring;
	typedef wstring::const_iterator const_iter;
public:
	explicit CMarkdown(const CHtmlRender &rndr);
	~CMarkdown();
	wstring tranform(const wstring &input);

protected:
	
	wstring preProcessing(const wstring &input,std::unordered_map<wstring, wstring> &ref_map);
	wstring stringToUpper(const wstring &input);

	void procAtxHeader(wstring &res, const_iter &beg, const_iter end, 
						std::unordered_map<wstring, wstring> &ref_map);
	void procHtmlLabel(wstring &res, const_iter &beg, const_iter end);
	void procBlockquote(wstring &res, const_iter &beg, const_iter end);
	void procOlist(wstring &res, const_iter &beg, const_iter end, size_t first_list_item_prefix,
					std::unordered_map<wstring, wstring> &ref_map);
	void procUlist(wstring &res, const_iter &beg, const_iter end, size_t first_list_item_prefix,
					std::unordered_map<wstring, wstring> &ref_map);
	void procListItem(wstring &res, const_iter beg, const_iter end, bool has_empty_line,
						size_t prefix_len, std::unordered_map<wstring, wstring> &ref_map);
	void procCode(wstring &res, const_iter &beg, const_iter end);
	void procParagraph(wstring &res, const_iter &beg, const_iter end,
						std::unordered_map<wstring, wstring> &ref_map);
	wstring procInline(const_iter beg, const_iter end, std::unordered_map<wstring, wstring> &ref_map);
	wstring procInline(const wstring &input, std::unordered_map<wstring, wstring> &ref_map);
	

	bool is_atxheader(const_iter beg, const_iter end);
	size_t is_setextheader(const_iter beg, const_iter end);
	bool is_code(const_iter beg, const_iter end);
	size_t is_parting_line(const_iter beg, const_iter end);
    size_t empty_line(const_iter beg, const_iter end);
    size_t is_empty(const_iter beg, const_iter end);
	size_t is_blockquote(const_iter beg, const_iter end);
	size_t is_olist(const_iter beg, const_iter end);
	size_t actual_prefix_olist(const_iter beg, const_iter end);
	size_t is_ulist(const_iter beg, const_iter end);
	bool is_html_label(const_iter beg, const_iter end);
	
private:
	CHtmlRender render;
};

#endif