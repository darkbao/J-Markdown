#include "CHtmlRender.h"
typedef std::wstring wstring;

wstring CHtmlRender::head[6] = {L"<h1>",L"<h2>", L"<h3>", L"<h4>", L"<h5>", L"<h6>"};
wstring CHtmlRender::_head[6] = { L"</h1>", L"</h2>", L"</h3>", L"</h4>", L"</h5>", L"</h6>" };
wstring CHtmlRender::li = L"<li>", CHtmlRender::_li = L"</li>", 
		CHtmlRender::ul = L"<ul>", CHtmlRender::_ul = L"</ul>",
		CHtmlRender::ol = L"<ol>", CHtmlRender::_ol = L"</ol>";
wstring CHtmlRender::p = L"<p>", CHtmlRender::_p = L"</p>";
wstring CHtmlRender::pcode = L"<pre><code>", CHtmlRender::_pcode = L"</code></pre>",
		CHtmlRender::code = L"<code>", CHtmlRender::_code = L"</code>";
wstring CHtmlRender::blockquote = L"<blockquote>", CHtmlRender::_blockquote = L"</blockquote>";
wstring CHtmlRender::hr = L"<hr>", CHtmlRender::em = L"<em>", CHtmlRender::_em = L"</em>",
		CHtmlRender::strong = L"<strong>", CHtmlRender::_strong = L"</strong>",
		CHtmlRender::br = L"<br />";
CHtmlRender::CHtmlRender()
{
}

CHtmlRender::~CHtmlRender()
{
}

void CHtmlRender::renderHeader(wstring & res, const wstring &title,size_t level){
	if (level<=6)
		res += head[level - 1] + title + _head[level - 1];
	if (level>6)
		res += head[5] + title + _head[5];
}

void CHtmlRender::renderSetextHeader(wstring &res, const wstring &title, size_t level){
	if (level == 1)
		res += head[0] + title + _head[0];
	else
		res += head[1] + title + _head[1];
}

void CHtmlRender::renderParagraph(wstring & res, wstring &transformed_content){
	if (transformed_content.empty())
		return;
	res += p + transformed_content + _p;
}

void CHtmlRender::renderOlist(wstring & res, CHtmlRender::position p){
	if (p == front)
		res += ol;
	else
		res += _ol;
}

void CHtmlRender::renderUlist(wstring & res, CHtmlRender::position p){
	if (p == front)
		res += ul;
	else
		res += _ul;
}

void CHtmlRender::renderListItem(wstring & res, CHtmlRender::position p){
	if (p == front)
		res += li;
	else
		res += _li;
}

void CHtmlRender::renderBlockquote(wstring & res,wstring & transformed_content){
	res += blockquote + transformed_content + _blockquote;
}

void CHtmlRender::renderCode(wstring & res,const wstring & transformed_code){
	res += pcode + transformed_code + _pcode;
}

void  CHtmlRender::renderInlineCode(wstring &res, const wstring &transformed_code){
	res += code + transformed_code + _code;
}

void CHtmlRender::renderSeparator(wstring &res){
	res += hr;
}

void CHtmlRender::renderEmphasis(wstring &res, const wstring &transformed_content, emphasis_type t){
	if (t==em_t)
		res += em + transformed_content + _em;
	else if (t==str_t)
		res += strong + transformed_content + _strong;
	else
		res += strong + em + transformed_content + _em + _strong;
}

void CHtmlRender::renderImage(wstring &res, const wstring &first, const wstring &second){
	bool has_rendered = false;
	if (!second.empty())
	{
		auto pos1 = second.find(L'\"');
		if (pos1 != wstring::npos && second.cbegin() + pos1 + 1 != second.cend())
		{
			auto pos2 = second.find(L'\"', pos1 + 1);
			if (pos2 != wstring::npos){
				res += (L"<img src = \"" + wstring(second.cbegin(), second.cbegin() + pos1) + L"\" alt = \""
					+ first + L"\" title = \"" + wstring(second.cbegin() + pos1 + 1, second.cbegin() + pos2) + L"\">");
				has_rendered = true;
			}
		}
	}
	if (!has_rendered)
		res += (L"<img src = \"" + second + L"\" alt = \"" + first + L"\">");
}

void CHtmlRender::renderLink(wstring &res, const wstring &first, const wstring &second){
	bool has_rendered = false;
	if (!second.empty())
	{
		auto pos1 = second.find(L'\"');
		if (pos1 != wstring::npos && second.cbegin() + pos1 + 1 != second.cend())
		{
			auto pos2 = second.find(L'\"', pos1 + 1);
			if (pos2 != wstring::npos){
				res += (L"<a href = \"" + wstring(second.cbegin(), second.cbegin() + pos1) 
					 + L"\" title = \"" + wstring(second.cbegin() + pos1 + 1, second.cbegin() + pos2) + L"\">"
					 + first + L"</a>");
				has_rendered = true;
			}
		}
	}
	if (!has_rendered)
		res += (L"<a href = \"" + second + L"\">" + first + L"</a>");
}


void CHtmlRender::tranSpecialchar(const wchar_t wc,wstring &input){
	if (wc == L'&')
		input += L"&amp;";
	else if (wc == L'<')
		input += L"&lt;";
	else if (wc == L'>')
		input += L"&gt;";
	else
		input += wc;
}

wstring CHtmlRender::tranSpecialchar(const wstring &input){
	wstring tmp;
	for (auto it = input.begin(); it !=input.end(); ++it){
		tranSpecialchar(*it, tmp);
	}
	return tmp;
}

void CHtmlRender::renderLineBreak(wstring &input){
	input += br;
}

