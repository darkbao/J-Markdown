#ifndef _CHTMLRENDER_H
#define _CHTMLRENDER_H
#include <string>
class CHtmlRender
{
	typedef std::wstring wstring;
	typedef wstring::const_iterator const_iter;
	
public:
	enum position{ front, rear };
	enum emphasis_type{ em_t, str_t, both };
	CHtmlRender();
	~CHtmlRender();
	static void renderHeader(wstring &res, const wstring &title, size_t level);
	static void renderSetextHeader(wstring &res, const wstring &title, size_t level);
	static void renderParagraph(wstring &res, wstring &transformed_content);
	static void renderUlist(wstring &res, position p);
	static void renderOlist(wstring &res, position p);
	static void renderListItem(wstring &res,position p);
	static void renderBlockquote(wstring &res, wstring &transformed_content);
	static void renderCode(wstring &res,const wstring &transformed_code);
	static void renderInlineCode(wstring &res, const wstring &transformed_code);
	static void renderSeparator(wstring &res);
	static void renderEmphasis(wstring &res,const wstring &transformed_content,emphasis_type t);
	static void renderImage(wstring &res,const wstring &first,const wstring &second);
	static void renderLink(wstring &res, const wstring &first, const wstring &second);
	static void tranSpecialchar(wchar_t wc,wstring &input);
	static wstring tranSpecialchar(const wstring &input);
	static void renderLineBreak(wstring &input);

protected:
	static wstring head[6];
	static wstring _head[6];
	static wstring li, _li, ul, _ul, ol, _ol;
	static wstring p, _p;
	static wstring pcode, _pcode, code, _code;
	static wstring blockquote, _blockquote;
	static wstring hr,em,_em,strong,_strong,br;
	static wstring html_head, html_tail;
};


#endif