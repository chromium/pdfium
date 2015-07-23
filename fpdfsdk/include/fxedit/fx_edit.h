// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FXEDIT_FX_EDIT_H_
#define FPDFSDK_INCLUDE_FXEDIT_FX_EDIT_H_

#include "../../../core/include/fxcrt/fx_basic.h"
#include "../../../core/include/fxge/fx_dib.h"

class CFX_RenderDevice;
class CPDF_Font;
class CPDF_Matrix;
class CPDF_PageObjects;
class CPDF_Point;
class CPDF_Rect;
class CPDF_TextObject;
class IFX_Edit;
class IFX_Edit_FontMap;
class IFX_Edit_Iterator;
class IFX_Edit_Notify;
class IFX_Edit_UndoItem;
class IFX_List;
class IFX_List_Notify;
class IFX_SystemHandler;
class IPDF_VariableText;
class IPDF_VariableText_Provider;
struct CPVT_Line;
struct CPVT_SecProps;
struct CPVT_Section;
struct CPVT_Word;
struct CPVT_WordPlace;
struct CPVT_WordProps;
struct CPVT_WordRange;

#define PVTWORD_STYLE_NORMAL				0x0000L
#define PVTWORD_STYLE_HIGHLIGHT				0x0001L
#define PVTWORD_STYLE_UNDERLINE				0x0002L
#define PVTWORD_STYLE_CROSSOUT				0x0004L
#define PVTWORD_STYLE_SQUIGGLY				0x0008L
#define PVTWORD_STYLE_DUALCROSSOUT			0x0010L
#define PVTWORD_STYLE_BOLD					0x0020L
#define PVTWORD_STYLE_ITALIC				0x0040L

#define FX_EDIT_ISLATINWORD(u)	(u == 0x2D || (u <= 0x005A && u >= 0x0041) || (u <= 0x007A && u >= 0x0061) || (u <= 0x02AF && u >= 0x00C0))

#ifdef FX_READER_DLL
	#ifdef FXET_EXPORT
		#define FXET_CLASS __declspec(dllexport)
	#else
		#define FXET_CLASS
	#endif
#else
	#define FXET_CLASS
#endif

#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET         1
#endif

class IFX_Edit_FontMap
{
public:
        virtual ~IFX_Edit_FontMap() { }
	//map a fontindex to pdf font.
	virtual CPDF_Font *						GetPDFFont(int32_t nFontIndex) = 0;
	//get the alias of a pdf font.
	virtual CFX_ByteString					GetPDFFontAlias(int32_t nFontIndex) = 0;
	//get the index of a font that can show a word.
	virtual int32_t						GetWordFontIndex(FX_WORD word, int32_t charset, int32_t nFontIndex) = 0;
	//get the charcode of word from unicode
	virtual int32_t						CharCodeFromUnicode(int32_t nFontIndex, FX_WORD word) = 0;
	//get the charset of unicode
	virtual int32_t						CharSetFromUnicode(FX_WORD word, int32_t nOldCharset) = 0;
};

class IFX_Edit_Notify
{
public:
        virtual ~IFX_Edit_Notify() { }
	//set the horizontal scrollbar information.
	virtual void							IOnSetScrollInfoX(FX_FLOAT fPlateMin, FX_FLOAT fPlateMax,
												FX_FLOAT fContentMin, FX_FLOAT fContentMax,
												FX_FLOAT fSmallStep, FX_FLOAT fBigStep) = 0;
	//set the vertical scrollbar information.
	virtual void							IOnSetScrollInfoY(FX_FLOAT fPlateMin, FX_FLOAT fPlateMax,
												FX_FLOAT fContentMin, FX_FLOAT fContentMax,
												FX_FLOAT fSmallStep, FX_FLOAT fBigStep) = 0;
	//set the position of horizontal scrollbar.
	virtual void							IOnSetScrollPosX(FX_FLOAT fx) = 0;
	//set the position of vertical scrollbar.
	virtual void							IOnSetScrollPosY(FX_FLOAT fy) = 0;
	//set the caret information.
	virtual void							IOnSetCaret(bool bVisible,const CPDF_Point & ptHead,const CPDF_Point & ptFoot, const CPVT_WordPlace& place) = 0;
	//if the caret position is changed ,send the information of current postion to user.
	virtual void							IOnCaretChange(const CPVT_SecProps & secProps, const CPVT_WordProps & wordProps) = 0;
	//if the text area is changed, send the information to user.
	virtual void							IOnContentChange(const CPDF_Rect& rcContent) = 0;
	//Invalidate the rectangle relative to the bounding box of edit.
	virtual void							IOnInvalidateRect(CPDF_Rect * pRect) = 0;
};

class IFX_Edit_OprNotify
{
public:
        virtual ~IFX_Edit_OprNotify() { }

	//OprType: 0
	virtual void							OnInsertWord(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace) = 0;
	//OprType: 1
	virtual void							OnInsertReturn(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace) = 0;
	//OprType: 2
	virtual void							OnBackSpace(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace) = 0;
	//OprType: 3
	virtual void							OnDelete(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace) = 0;
	//OprType: 4
	virtual void							OnClear(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace) = 0;
	//OprType: 5
	virtual void							OnInsertText(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace) = 0;
	//OprType: 6
	virtual void							OnSetText(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace) = 0;
	//
	virtual void							OnAddUndo(IFX_Edit_UndoItem* pUndoItem) = 0;
};

class IFX_Edit_Iterator
{
public:
	virtual ~IFX_Edit_Iterator()  {}
public:
	//move the current position to the next word.
	virtual bool							NextWord() = 0;
	//move the current position to the next line.
	virtual bool							NextLine() = 0;
	//move the current position to the next section.
	virtual bool							NextSection() = 0;

	//move the current position to the previous word.
	virtual bool							PrevWord() = 0;
	//move the current position to the previous line.
	virtual bool							PrevLine() = 0;
	//move the current position to the previous section.
	virtual bool							PrevSection() = 0;

	//get the information of the current word.
	virtual bool							GetWord(CPVT_Word & word) const = 0;
	//get the information of the current line.
	virtual bool							GetLine(CPVT_Line & line) const = 0;
	//get the information of the current section.
	virtual bool							GetSection(CPVT_Section & section) const = 0;
	//set the current position.
	virtual void							SetAt(int32_t nWordIndex) = 0;
	//set the current position.
	virtual void							SetAt(const CPVT_WordPlace & place) = 0;
	//get the current position.
	virtual const CPVT_WordPlace &			GetAt() const = 0;

	//get the edit which this iterator belongs to
	virtual IFX_Edit*						GetEdit() const = 0;
};

class IFX_Edit_UndoItem
{
public:
	virtual ~IFX_Edit_UndoItem() { }

	virtual void							Undo() = 0;
	virtual void							Redo() = 0;
	virtual CFX_WideString					GetUndoTitle() = 0;
};

class FXET_CLASS IFX_Edit
{
public:
	static IFX_Edit*						NewEdit();
	static	void							DelEdit(IFX_Edit* pEdit);

        //set a IFX_Edit_FontMap pointer implemented by user.
	virtual void							SetFontMap(IFX_Edit_FontMap* pFontMap) = 0;
	//if user don't like to use FontMap, implement VTProvider and set it directly.
	virtual void							SetVTProvider(IPDF_VariableText_Provider* pProvider) = 0;
	//set a IFX_Edit_Notify pointer implemented by user.
	virtual void							SetNotify(IFX_Edit_Notify * pNotify) = 0;
	virtual void							SetOprNotify(IFX_Edit_OprNotify* pOprNotify) = 0;
	//get a pointer allocated by CPDF_Edit, by this pointer, user can iterate the contents of edit, but don't need to release.
	virtual IFX_Edit_Iterator*				GetIterator() = 0;
	//get a VT pointer relative to this edit.
	virtual IPDF_VariableText*				GetVariableText() = 0;
	//get the IFX_Edit_FontMap pointer set by user.
	virtual IFX_Edit_FontMap*				GetFontMap() = 0;

	//initialize the edit.
	virtual void							Initialize() = 0;

	//set the bounding box of the text area.
	virtual void							SetPlateRect(const CPDF_Rect & rect, bool bPaint = true) = 0;
	//set the scroll origin
	virtual void							SetScrollPos(const CPDF_Point & point) = 0;

	//set the horizontal text alignment in text box, nFormat (0:left 1:middle 2:right).
	virtual void							SetAlignmentH(int32_t nFormat = 0, bool bPaint = true) = 0;
	//set the vertical text alignment in text box, nFormat (0:top 1:center 2:bottom).
	virtual void							SetAlignmentV(int32_t nFormat = 0, bool bPaint = true) = 0;
	//if the text is shown in secret , set a character for substitute.
	virtual void							SetPasswordChar(FX_WORD wSubWord = '*', bool bPaint = true) = 0;
	//set the maximal count of words of the text.
	virtual void							SetLimitChar(int32_t nLimitChar = 0, bool bPaint = true) = 0;
	//if set the count of charArray , then all words is shown in equal space.
	virtual void							SetCharArray(int32_t nCharArray = 0, bool bPaint = true) = 0;
	//set the space of two characters.
	virtual void							SetCharSpace(FX_FLOAT fCharSpace = 0.0f, bool bPaint = true) = 0;
	//set the horizontal scale of all characters.
	virtual void							SetHorzScale(int32_t nHorzScale = 100, bool bPaint = true) = 0;
	//set the leading of all lines
	virtual void							SetLineLeading(FX_FLOAT fLineLeading, bool bPaint = true) = 0;
	//if set, CRLF is allowed.
	virtual void							SetMultiLine(bool bMultiLine = true, bool bPaint = true) = 0;
	//if set, all words auto fit the width of the bounding box.
	virtual void							SetAutoReturn(bool bAuto = true, bool bPaint = true) = 0;
	//if set, a font size is calculated to full fit the bounding box.
	virtual void							SetAutoFontSize(bool bAuto = true, bool bPaint = true) = 0;
	//is set, the text is allowed to scroll.
	virtual void							SetAutoScroll(bool bAuto = true, bool bPaint = true) = 0;
	//set the font size of all words.
	virtual void							SetFontSize(FX_FLOAT fFontSize, bool bPaint = true) = 0;
	//the text is allowed to auto-scroll, allow the text overflow?
	virtual void							SetTextOverflow(bool bAllowed = false, bool bPaint = true) = 0;

	//query if the edit is richedit.
	virtual bool							IsRichText() const = 0;
	//set the edit is richedit.
	virtual void							SetRichText(bool bRichText = true, bool bPaint = true) = 0;
	//set the fontsize of selected text.
	virtual bool							SetRichFontSize(FX_FLOAT fFontSize) = 0;
	//set the fontindex of selected text, user can change the font of selected text.
	virtual bool							SetRichFontIndex(int32_t nFontIndex) = 0;
	//set the textcolor of selected text.
	virtual bool							SetRichTextColor(FX_COLORREF dwColor) = 0;
	//set the text script type of selected text. (0:normal 1:superscript 2:subscript)
	virtual bool							SetRichTextScript(int32_t nScriptType) = 0;
	//set the bold font style of selected text.
	virtual bool							SetRichTextBold(bool bBold = true) = 0;
	//set the italic font style of selected text.
	virtual bool							SetRichTextItalic(bool bItalic = true) = 0;
	//set the underline style of selected text.
	virtual bool							SetRichTextUnderline(bool bUnderline = true) = 0;
	//set the crossout style of selected text.
	virtual bool							SetRichTextCrossout(bool bCrossout = true) = 0;
	//set the charspace of selected text, in user coordinate.
	virtual	bool							SetRichTextCharSpace(FX_FLOAT fCharSpace) = 0;
	//set the horizontal scale of selected text, default value is 100.
	virtual bool							SetRichTextHorzScale(int32_t nHorzScale = 100) = 0;
	//set the leading of selected section, in user coordinate.
	virtual bool							SetRichTextLineLeading(FX_FLOAT fLineLeading) = 0;
	//set the indent of selected section, in user coordinate.
	virtual bool							SetRichTextLineIndent(FX_FLOAT fLineIndent) = 0;
	//set the alignment of selected section, nAlignment(0:left 1:middle 2:right)
	virtual bool							SetRichTextAlignment(int32_t nAlignment) = 0;

	//set the selected range of text.
	//if nStartChar == 0 and nEndChar == -1, select all the text.
	virtual void							SetSel(int32_t nStartChar,int32_t nEndChar) = 0;
	//get the selected range of text.
	virtual void							GetSel(int32_t & nStartChar, int32_t & nEndChar) const = 0;
	//select all the text.
	virtual	void							SelectAll() = 0;
	//set text is not selected.
	virtual void							SelectNone() = 0;
	//get the caret position.
	virtual int32_t						GetCaret() const = 0;
	virtual CPVT_WordPlace					GetCaretWordPlace() const = 0;
	//get the string of selected text.
	virtual CFX_WideString					GetSelText() const = 0;
	//get the text conent
	virtual CFX_WideString					GetText() const = 0;
	//query if any text is selected.
	virtual bool							IsSelected() const = 0;
	//get the scroll origin
	virtual CPDF_Point						GetScrollPos() const = 0;
	//get the bounding box of the text area.
	virtual CPDF_Rect						GetPlateRect() const = 0;
	//get the fact area of the text.
	virtual CPDF_Rect						GetContentRect() const = 0;
	//get the visible word range
	virtual CPVT_WordRange					GetVisibleWordRange() const = 0;
	//get the whole word range
	virtual CPVT_WordRange					GetWholeWordRange() const = 0;
	//get the word range of select text
	virtual CPVT_WordRange					GetSelectWordRange() const = 0;

	//send the mousedown message to edit for response.
	//if Shift key is hold, bShift is true, is Ctrl key is hold, bCtrl is true.
	virtual void							OnMouseDown(const CPDF_Point & point,bool bShift,bool bCtrl) = 0;
	//send the mousemove message to edit when mouse down is true.
	virtual void							OnMouseMove(const CPDF_Point & point,bool bShift,bool bCtrl) = 0;
	//send the UP key message to edit.
	virtual void							OnVK_UP(bool bShift,bool bCtrl) = 0;
	//send the DOWN key message to edit.
	virtual void							OnVK_DOWN(bool bShift,bool bCtrl) = 0;
	//send the LEFT key message to edit.
	virtual void							OnVK_LEFT(bool bShift,bool bCtrl) = 0;
	//send the RIGHT key message to edit.
	virtual void							OnVK_RIGHT(bool bShift,bool bCtrl) = 0;
	//send the HOME key message to edit.
	virtual void							OnVK_HOME(bool bShift,bool bCtrl) = 0;
	//send the END key message to edit.
	virtual void							OnVK_END(bool bShift,bool bCtrl) = 0;

	//put text into edit.
	virtual void							SetText(const FX_WCHAR* text,int32_t charset = DEFAULT_CHARSET,
													const CPVT_SecProps * pSecProps = NULL,const CPVT_WordProps * pWordProps = NULL) = 0;
	//insert a word into the edit.
	virtual bool							InsertWord(FX_WORD word, int32_t charset = DEFAULT_CHARSET, const CPVT_WordProps * pWordProps = NULL) = 0;
	//insert a return into the edit.
	virtual bool							InsertReturn(const CPVT_SecProps * pSecProps = NULL,const CPVT_WordProps * pWordProps = NULL) = 0;
	//insert text into the edit.
	virtual bool							InsertText(const FX_WCHAR* text, int32_t charset = DEFAULT_CHARSET,
													const CPVT_SecProps * pSecProps = NULL,const CPVT_WordProps * pWordProps = NULL) = 0;
	//do backspace operation.
	virtual bool							Backspace() = 0;
	//do delete operation.
	virtual bool							Delete() = 0;
	//delete the selected text.
	virtual bool							Clear() = 0;

	//do Redo operation.
	virtual bool							Redo() = 0;
	//do Undo operation.
	virtual bool							Undo() = 0;
	//move caret
	virtual void							SetCaret(int32_t nPos) = 0;

	//arrange all words over again
	virtual void							Paint() = 0;

	//allow to refresh screen?
	virtual void							EnableRefresh(bool bRefresh) = 0;

	virtual void							RefreshWordRange(const CPVT_WordRange& wr) = 0;

	//allow undo/redo?
	virtual void							EnableUndo(bool bUndo) = 0;

	//allow notify?
	virtual void							EnableNotify(bool bNotify) = 0;

	//allow opr notify?
	virtual void							EnableOprNotify(bool bNotify) = 0;

	//map word place to word index.
	virtual int32_t						WordPlaceToWordIndex(const CPVT_WordPlace & place) const = 0;
	//map word index to word place.
	virtual CPVT_WordPlace					WordIndexToWordPlace(int32_t index) const = 0;

	//get the beginning position of a line
	virtual CPVT_WordPlace					GetLineBeginPlace(const CPVT_WordPlace & place) const = 0;

	//get the ending position of a line
	virtual CPVT_WordPlace					GetLineEndPlace(const CPVT_WordPlace & place) const = 0;

	//get the beginning position of a section
	virtual CPVT_WordPlace					GetSectionBeginPlace(const CPVT_WordPlace & place) const = 0;

	//get the ending position of a section
	virtual CPVT_WordPlace					GetSectionEndPlace(const CPVT_WordPlace & place) const = 0;

	//search a wordplace form point
	virtual CPVT_WordPlace					SearchWordPlace(const CPDF_Point& point) const = 0;

	//get the font size of non_rich text or default font size of richtext.
	virtual FX_FLOAT						GetFontSize() const = 0;
	//get the mask character.
	virtual FX_WORD							GetPasswordChar() const = 0;
	//get the count of charArray
	virtual int32_t						GetCharArray() const = 0;
	//get the horizontal scale of all characters
	virtual int32_t						GetHorzScale() const = 0;
	//get the space of two characters
	virtual FX_FLOAT						GetCharSpace() const = 0;
	//get the latin words of specified range
	virtual CFX_WideString					GetRangeText(const CPVT_WordRange & range) const = 0;
	//is the text full in bounding box
	virtual bool							IsTextFull() const = 0;
	virtual bool							CanUndo() const = 0;
	virtual bool							CanRedo() const = 0;
	//if the content is changed after settext?
	virtual bool							IsModified() const = 0;
	//get the total words in edit
	virtual int32_t						GetTotalWords() const = 0;

	virtual void							AddUndoItem(IFX_Edit_UndoItem* pUndoItem) = 0;

	static CFX_ByteString					GetEditAppearanceStream(IFX_Edit* pEdit, const CPDF_Point & ptOffset,
													const CPVT_WordRange* pRange = NULL,
													bool bContinuous = true, FX_WORD SubWord = 0);
	static CFX_ByteString					GetSelectAppearanceStream(IFX_Edit* pEdit, const CPDF_Point & ptOffset, const CPVT_WordRange* pRange = NULL);
	static void								DrawEdit(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device, IFX_Edit* pEdit, FX_COLORREF crTextFill, FX_COLORREF crTextStroke,
													const CPDF_Rect& rcClip, const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, IFX_SystemHandler* pSystemHandler, void* pFFLData);
	static void								DrawUnderline(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device, IFX_Edit* pEdit, FX_COLORREF color,
													const CPDF_Rect& rcClip, const CPDF_Point& ptOffset, const CPVT_WordRange* pRange);
	static void								DrawRichEdit(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device, IFX_Edit* pEdit,
													const CPDF_Rect& rcClip, const CPDF_Point& ptOffset, const CPVT_WordRange* pRange);
	static void								GeneratePageObjects(CPDF_PageObjects* pPageObjects, IFX_Edit* pEdit,
													const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, FX_COLORREF crText, CFX_ArrayTemplate<CPDF_TextObject*>& ObjArray);
	static void								GenerateRichPageObjects(CPDF_PageObjects* pPageObjects, IFX_Edit* pEdit,
													const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, CFX_ArrayTemplate<CPDF_TextObject*>& ObjArray);
	static void								GenerateUnderlineObjects(CPDF_PageObjects* pPageObjects, IFX_Edit* pEdit,
													const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, FX_COLORREF color);

protected:
    ~IFX_Edit() { }
};

class IFX_List_Notify
{
public:
        virtual ~IFX_List_Notify() { }
	//set the horizontal scrollbar information.
	virtual void							IOnSetScrollInfoX(FX_FLOAT fPlateMin, FX_FLOAT fPlateMax,
												FX_FLOAT fContentMin, FX_FLOAT fContentMax,
												FX_FLOAT fSmallStep, FX_FLOAT fBigStep) = 0;
	//set the vertical scrollbar information.
	virtual void							IOnSetScrollInfoY(FX_FLOAT fPlateMin, FX_FLOAT fPlateMax,
												FX_FLOAT fContentMin, FX_FLOAT fContentMax,
												FX_FLOAT fSmallStep, FX_FLOAT fBigStep) = 0;
	//set the position of horizontal scrollbar.
	virtual void							IOnSetScrollPosX(FX_FLOAT fx) = 0;
	//set the position of vertical scrollbar.
	virtual void							IOnSetScrollPosY(FX_FLOAT fy) = 0;
	//Invalidate the rectangle relative to the bounding box of edit.
	virtual void							IOnInvalidateRect(CPDF_Rect * pRect) = 0;
};

class FXET_CLASS IFX_List
{
public:
	static IFX_List*						NewList();
	static void								DelList(IFX_List* pList);

	virtual void							SetFontMap(IFX_Edit_FontMap * pFontMap) = 0;
	virtual void							SetNotify(IFX_List_Notify * pNotify) = 0;

	virtual void							SetPlateRect(const CPDF_Rect & rect) = 0;
	virtual void							SetFontSize(FX_FLOAT fFontSize) = 0;

	virtual CPDF_Rect						GetPlateRect() const = 0;
	virtual CPDF_Rect						GetContentRect() const = 0;

	virtual FX_FLOAT						GetFontSize() const = 0;
	virtual IFX_Edit*						GetItemEdit(int32_t nIndex) const = 0;
	virtual int32_t						GetCount() const = 0;
	virtual bool							IsItemSelected(int32_t nIndex) const = 0;
	virtual FX_FLOAT						GetFirstHeight() const = 0;

	virtual void							SetMultipleSel(bool bMultiple) = 0;
	virtual bool							IsMultipleSel() const = 0;
	virtual bool							IsValid(int32_t nItemIndex) const = 0;
	virtual int32_t						FindNext(int32_t nIndex,FX_WCHAR nChar) const = 0;

	virtual void							SetScrollPos(const CPDF_Point & point) = 0;
	virtual void							ScrollToListItem(int32_t nItemIndex) = 0;
	virtual CPDF_Rect						GetItemRect(int32_t nIndex) const = 0;
	virtual int32_t						GetCaret() const = 0;
	virtual int32_t						GetSelect() const = 0;
	virtual int32_t						GetTopItem() const = 0;
	virtual int32_t						GetItemIndex(const CPDF_Point & point) const = 0;
	virtual int32_t						GetFirstSelected() const = 0;

	virtual void							AddString(const FX_WCHAR* string) = 0;
	virtual void							SetTopItem(int32_t nIndex) = 0;
	virtual void							Select(int32_t nItemIndex) = 0;
	virtual void							SetCaret(int32_t nItemIndex) = 0;
	virtual void							Empty() = 0;
	virtual void							Cancel() = 0;
	virtual CFX_WideString					GetText() const = 0;


	virtual void							OnMouseDown(const CPDF_Point & point,bool bShift,bool bCtrl) = 0;
	virtual void							OnMouseMove(const CPDF_Point & point,bool bShift,bool bCtrl) = 0;
	virtual void							OnVK_UP(bool bShift,bool bCtrl) = 0;
	virtual void							OnVK_DOWN(bool bShift,bool bCtrl) = 0;
	virtual void							OnVK_LEFT(bool bShift,bool bCtrl) = 0;
	virtual void							OnVK_RIGHT(bool bShift,bool bCtrl) = 0;
	virtual void							OnVK_HOME(bool bShift,bool bCtrl) = 0;
	virtual void							OnVK_END(bool bShift,bool bCtrl) = 0;
	virtual void							OnVK(int32_t nItemIndex,bool bShift,bool bCtrl) = 0;
	virtual bool							OnChar(FX_WORD nChar,bool bShift,bool bCtrl) = 0;

protected:
        ~IFX_List() { }
};

#endif  // FPDFSDK_INCLUDE_FXEDIT_FX_EDIT_H_
