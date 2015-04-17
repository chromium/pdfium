// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "layoutprovider_taggedpdf.h"
CPDF_LayoutElement::CPDF_LayoutElement()
{
    m_pTaggedElement = NULL;
    m_pParentElement = NULL;
}
CPDF_LayoutElement::~CPDF_LayoutElement()
{
    m_ObjArray.RemoveAll();
    int size = m_ChildArray.GetSize();
    for(int i = 0; i < size; i++) {
        CPDF_LayoutElement* pChild = (CPDF_LayoutElement*)m_ChildArray.GetAt(i);
        delete pChild;
        pChild = NULL;
    }
    m_ChildArray.RemoveAll();
}
LayoutType CPDF_LayoutElement::ConvertLayoutType(FX_BSTR name)
{
    if(name == (const char*)("Document")) {
        return LayoutDocument;
    } else if(name == (const char*)("Part")) {
        return LayoutPart;
    } else if(name == (const char*)("Art")) {
        return LayoutArt;
    } else if(name == (const char*)("Sect")) {
        return LayoutSect;
    } else if(name == (const char*)("Div")) {
        return LayoutDiv;
    } else if(name == (const char*)("BlockQuote")) {
        return LayoutBlockQuote;
    } else if(name == (const char*)("Caption")) {
        return LayoutCaption;
    } else if(name == (const char*)("TOC")) {
        return LayoutTOC;
    } else if(name == (const char*)("TOCI")) {
        return LayoutTOCI;
    } else if(name == (const char*)("Index")) {
        return LayoutIndex;
    } else if(name == (const char*)("NonStruct")) {
        return LayoutNonStruct;
    } else if(name == (const char*)("Private")) {
        return LayoutPrivate;
    } else if(name == (const char*)("P")) {
        return LayoutParagraph;
    } else if(name == (const char*)("H")) {
        return LayoutHeading;
    } else if(name == (const char*)("H1")) {
        return LayoutHeading1;
    } else if(name == (const char*)("H2")) {
        return LayoutHeading2;
    } else if(name == (const char*)("H3")) {
        return LayoutHeading3;
    } else if(name == (const char*)("H4")) {
        return LayoutHeading4;
    } else if(name == (const char*)("H5")) {
        return LayoutHeading5;
    } else if(name == (const char*)("H6")) {
        return LayoutHeading6;
    } else if(name == (const char*)("L")) {
        return LayoutList;
    } else if(name == (const char*)("LI")) {
        return LayoutListItem;
    } else if(name == (const char*)("Lbl")) {
        return LayoutListLabel;
    } else if(name == (const char*)("LBody")) {
        return LayoutListBody;
    } else if(name == (const char*)("Table")) {
        return LayoutTable;
    } else if(name == (const char*)("TR")) {
        return LayoutTableRow;
    } else if(name == (const char*)("TH")) {
        return LayoutTableHeaderCell;
    } else if(name == (const char*)("TD")) {
        return LayoutTableDataCell;
    } else if(name == (const char*)("THead")) {
        return LayoutTableHeaderGroup;
    } else if(name == (const char*)("TBody")) {
        return LayoutTableBodyGroup;
    } else if(name == (const char*)("TFoot")) {
        return LayoutTableFootGroup;
    } else if(name == (const char*)("Span")) {
        return LayoutSpan;
    } else if(name == (const char*)("Quote")) {
        return LayoutQuote;
    } else if(name == (const char*)("Note")) {
        return LayoutNote;
    } else if(name == (const char*)("Reference")) {
        return LayoutReference;
    } else if(name == (const char*)("BibEntry")) {
        return LayoutBibEntry;
    } else if(name == (const char*)("Code")) {
        return LayoutCode;
    } else if(name == (const char*)("Link")) {
        return LayoutLink;
    } else if(name == (const char*)("Annot")) {
        return LayoutAnnot;
    } else if(name == (const char*)("Ruby")) {
        return LayoutRuby;
    } else if(name == (const char*)("RB")) {
        return LayoutRubyBase;
    } else if(name == (const char*)("RT")) {
        return LayoutRubyAnnot;
    } else if(name == (const char*)("RP")) {
        return LayoutRubyPunc;
    } else if(name == (const char*)("Warichu")) {
        return LayoutWarichu;
    } else if(name == (const char*)("WT")) {
        return LayoutWarichuText;
    } else if(name == (const char*)("WP")) {
        return LayoutWarichuPunc;
    } else if(name == (const char*)("Figure")) {
        return LayoutFigure;
    } else if(name == (const char*)("Formula")) {
        return LayoutFormula;
    } else if(name == (const char*)("Form")) {
        return LayoutForm;
    } else {
        return LayoutUnknown;
    }
}
CFX_ByteStringC CPDF_LayoutElement::ConvertLayoutType(LayoutType type)
{
    FX_BSTR name = "";
    if(type == LayoutArifact) {
        return "Arifact";
    } else if( type == LayoutDocument) {
        return "Document";
    } else if( type == LayoutPart) {
        return "Part";
    } else if( type == LayoutArt) {
        return "Art";
    } else if( type == LayoutSect) {
        return "Sect";
    } else if( type == LayoutDiv) {
        return "Div";
    } else if( type == LayoutBlockQuote) {
        return "BlockQuote";
    } else if( type == LayoutCaption) {
        return "Caption";
    } else if( type == LayoutTOC) {
        return "TOC";
    } else if( type == LayoutTOCI) {
        return "TOCI";
    } else if( type == LayoutIndex) {
        return "Index";
    } else if( type == LayoutNonStruct) {
        return "NonStruct";
    } else if( type == LayoutPrivate) {
        return "Private";
    } else if( type == LayoutParagraph) {
        return "P";
    } else if( type == LayoutHeading) {
        return "H";
    } else if( type == LayoutHeading1) {
        return "H1";
    } else if( type == LayoutHeading2) {
        return "H2";
    } else if( type == LayoutHeading3) {
        return "H3";
    } else if( type == LayoutHeading4) {
        return "H4";
    } else if( type == LayoutHeading5) {
        return "H5";
    } else if( type == LayoutHeading6) {
        return "H6";
    } else if( type == LayoutList) {
        return "L";
    } else if( type == LayoutListItem) {
        return "LI";
    } else if( type == LayoutListLabel) {
        return "Lbl";
    } else if( type == LayoutListBody) {
        return "LBody";
    } else if( type == LayoutTable) {
        return "Table";
    } else if( type == LayoutTableRow) {
        return "TR";
    } else if( type == LayoutTableHeaderCell) {
        return "TH";
    } else if( type == LayoutTableDataCell) {
        return "TD";
    } else if( type == LayoutTableHeaderGroup) {
        return "THead";
    } else if( type == LayoutTableBodyGroup) {
        return "TBody";
    } else if( type == LayoutTableFootGroup) {
        return "TFoot";
    } else if( type == LayoutSpan) {
        return "Span";
    } else if( type == LayoutQuote) {
        return "Quote";
    } else if( type == LayoutNote) {
        return "Note";
    } else if( type == LayoutReference) {
        return "Reference";
    } else if( type == LayoutBibEntry) {
        return "BibEntry";
    } else if( type == LayoutCode) {
        return "Code";
    } else if( type == LayoutLink) {
        return "Link";
    } else if( type == LayoutAnnot) {
        return "Annot";
    } else if( type == LayoutRuby) {
        return "Ruby";
    } else if( type == LayoutRubyBase) {
        return "RB";
    } else if( type == LayoutRubyAnnot) {
        return "RT";
    } else if( type == LayoutRubyPunc) {
        return "RP";
    } else if( type == LayoutWarichu) {
        return "Warichu";
    } else if( type == LayoutWarichuText) {
        return "WT";
    } else if( type == LayoutWarichuPunc) {
        return "WP";
    } else if( type == LayoutFigure) {
        return "Figure";
    } else if( type == LayoutFormula) {
        return "Formula";
    } else if( type == LayoutForm) {
        return "Form";
    }
    return name;
}
CFX_ByteStringC CPDF_LayoutElement::ConvertLayoutAttr(LayoutAttr attr)
{
    switch(attr) {
        case LayoutArtifactType:
            return "Type";
        case LayoutArtifactAttached:
            return "Attached";
        case LayoutArtifactSubType:
            return "Subtype";
        case LayoutPlacement:
            return "Placement";
        case LayoutWritingMode:
            return "WritingMode";
        case LayoutBackgroundColor:
            return "BackgroundColor";
        case LayoutBorderColor:
            return "BorderColor";
        case LayoutBorderStyle:
            return "BorderStyle";
        case LayoutBorderThickness:
            return "BorderThickness";
        case LayoutPadding:
            return "Padding";
        case LayoutColor:
            return "Color";
        case LayoutSpaceBefore:
            return "SpaceBefore";
        case LayoutSpaceAfter:
            return "SpaceAfter";
        case LayoutStartIndent:
            return "StartIndent";
        case LayoutEndIndent:
            return "EndIndent";
        case LayoutTextIndent:
            return "TextIndent";
        case LayoutTextAlign:
            return "TextAlign";
        case LayoutBBox:
            return "BBox";
        case LayoutWidth:
            return "Width";
        case LayoutHeight:
            return "Height";
        case LayoutBlockAlign:
            return "BlockAlign";
        case LayoutInlineAlign:
            return "InlineAlign";
        case LayoutTBorderStyle:
            return "TBorderStyle";
        case LayoutTPadding:
            return "TPadding";
        case LayoutBaselineShift:
            return "BaselineShift";
        case LayoutLineHeight:
            return "LineHeight";
        case LayoutTextDecorationColor:
            return "TextDecorationColor";
        case LayoutTextDecorationThickness:
            return "TextDecorationThickness";
        case LayoutTextDecorationType:
            return "TextDecorationType";
        case LayoutRubyAlign:
            return "RubyAlign";
        case LayoutRubyPosition:
            return "RubyPosition";
        case LayoutGlyphOrientationVertical:
            return "GlyphOrientationVertical";
        case LayoutColumnCount:
            return "ColumnCount";
        case LayoutColumnGap:
            return "ColumnGap";
        case LayoutColumnWidths:
            return "ColumnWidths";
        case LayoutListNumbering:
            return "ListNumbering";
        case LayoutFieldRole:
            return "Role";
        case LayoutFieldChecked:
            return "checked";
        case LayoutFieldDesc:
            return "Desc";
        case LayoutRowSpan:
            return "RowSpan";
        case LayoutColSpan:
            return "ColSpan";
        case LayoutTableHeaders:
            return "Headers";
        case LayoutTableHeaderScope:
            return "Scope";
        case LayoutTableSummary:
            return "Summary";
        default:
            return "";
    }
}
LayoutEnum CPDF_LayoutElement::ConvertLayoutEnum(CFX_ByteStringC Enum)
{
    if(Enum == "Block") {
        return LayoutBlock;
    } else if (Enum == "Inline") {
        return LayoutInline;
    } else if (Enum == "Before") {
        return LayoutBefore;
    } else if (Enum == "Start") {
        return LayoutStart;
    } else if (Enum == "End") {
        return LayoutEnd;
    } else if (Enum == "LrTb") {
        return LayoutLrTb;
    } else if (Enum == "RlTb") {
        return LayoutRlTb;
    } else if (Enum == "TbRl") {
        return LayoutTbRl;
    } else if (Enum == "None") {
        return LayoutNone;
    } else if (Enum == "Hidden") {
        return LayoutHidden;
    } else if (Enum == "Dotted") {
        return LayoutDotted;
    } else if (Enum == "Dashed") {
        return LayoutDashed;
    } else if (Enum == "Solid") {
        return LayoutSolid;
    } else if (Enum == "Double") {
        return LayoutDouble;
    } else if (Enum == "Groove") {
        return LayoutGroove;
    } else if (Enum == "Ridge") {
        return LayoutRidge;
    } else if (Enum == "Inset") {
        return LayoutInset;
    } else if (Enum == "Outset") {
        return LayoutOutset;
    } else if (Enum == "Normal") {
        return LayoutNormal;
    } else if (Enum == "Auto") {
        return LayoutAuto;
    } else if (Enum == "Center") {
        return LayoutCenter;
    } else if (Enum == "Justify") {
        return LayoutJustify;
    } else if (Enum == "Middle") {
        return LayoutMiddle;
    } else if (Enum == "Underline") {
        return LayoutUnderline;
    } else if (Enum == "Overline") {
        return LayoutOverline;
    } else if (Enum == "LineThrough") {
        return LayoutLineThrough;
    } else if (Enum == "Distribute") {
        return LayoutDistribute;
    } else if (Enum == "Disc") {
        return LayoutDisc;
    } else if (Enum == "Circle") {
        return LayoutCircle;
    } else if (Enum == "Square") {
        return LayoutSquare;
    } else if (Enum == "Decimal") {
        return LayoutDecimal;
    } else if (Enum == "UpperRoman") {
        return LayoutUpperRoman;
    } else if (Enum == "LowerRoman") {
        return LayoutLowerRoman;
    } else if (Enum == "UpperAlpha") {
        return LayoutUpperAlpha;
    } else if (Enum == "LowerAlpha") {
        return LayoutLowerAlpha;
    } else if (Enum == "rb") {
        return LayoutRB;
    } else if (Enum == "cb") {
        return LayoutCB;
    } else if (Enum == "pb") {
        return LayoutPB;
    } else if (Enum == "tv") {
        return LayoutTV;
    } else if (Enum == "on") {
        return LayoutOn;
    } else if (Enum == "off") {
        return LayoutOff;
    } else if (Enum == "neutral") {
        return LayoutNeutral;
    } else if (Enum == "Row") {
        return LayoutRow;
    } else if (Enum == "Column") {
        return LayoutColumn;
    } else if (Enum == "Both") {
        return LayoutBoth;
    } else if (Enum == "Left") {
        return LayoutLeft;
    } else if (Enum == "Top") {
        return LayoutTop;
    } else if (Enum == "Bottom") {
        return LayoutBottom;
    } else if (Enum == "Right") {
        return LayoutRight;
    } else if (Enum == "Pagination") {
        return LayoutPagination;
    } else if (Enum == "Layout") {
        return LayoutLayout;
    } else if (Enum == "Page") {
        return LayoutPage;
    } else if (Enum == "Background") {
        return LayoutBackground;
    } else if (Enum == "Header") {
        return LayoutHeader;
    } else if (Enum == "Footer") {
        return LayoutFooter;
    } else if (Enum == "Watermark") {
        return LayoutWatermark;
    } else {
        return LayoutInvalid;
    }
}
LayoutType CPDF_LayoutElement::GetType()
{
    if(!m_pTaggedElement) {
        return LayoutUnknown;
    }
    CFX_ByteString name = m_pTaggedElement->GetType();
    return this->ConvertLayoutType(name);
}
int	CPDF_LayoutElement::CountAttrValues(LayoutAttr attr_type)
{
    if(!m_pTaggedElement) {
        return 0;
    }
    CPDF_Object* pObj = m_pTaggedElement->GetAttr(GetAttrOwner(attr_type), ConvertLayoutAttr(attr_type), IsInheritable(attr_type));
    if(pObj) {
        return 1;
    } else {
        return 0;
    }
}
LayoutEnum CPDF_LayoutElement::GetEnumAttr(LayoutAttr attr_type, int index)
{
    if(!m_pTaggedElement) {
        return LayoutInvalid;
    }
    CFX_ByteStringC owner = GetAttrOwner(attr_type);
    CFX_ByteStringC default_value = GetDefaultNameValue(attr_type);
    CFX_ByteStringC AttrName = ConvertLayoutAttr(attr_type);
    CFX_ByteString	AttrValue = m_pTaggedElement->GetName(owner, AttrName, default_value, IsInheritable(attr_type), index);
    return ConvertLayoutEnum(AttrValue);
}
CFX_ByteStringC CPDF_LayoutElement::GetAttrOwner(LayoutAttr attr_type)
{
    switch(attr_type) {
        case LayoutListNumbering:
            return "List";
        case LayoutFieldRole:
        case LayoutFieldChecked :
        case LayoutFieldDesc:
            return "PrintField";
        case LayoutRowSpan:
        case LayoutColSpan:
        case LayoutTableHeaders:
        case LayoutTableHeaderScope:
        case LayoutTableSummary:
            return "Table";
        default:
            return "Layout";
    }
}
FX_FLOAT	CPDF_LayoutElement::GetNumberAttr(LayoutAttr attr_type, int index)
{
    if(!m_pTaggedElement) {
        return 0;
    }
    CFX_ByteStringC owner = GetAttrOwner(attr_type);
    FX_FLOAT default_value = GetDefaultFloatValue(attr_type);
    CFX_ByteStringC AttrName = ConvertLayoutAttr(attr_type);
    FX_FLOAT f = m_pTaggedElement->GetNumber(owner, AttrName, default_value, IsInheritable(attr_type), index);
    if(attr_type == LayoutWidth && !f) {
        f = m_pTaggedElement->GetNumber("Table", AttrName, default_value, IsInheritable(attr_type), index);
    }
    return f;
}
FX_COLORREF	CPDF_LayoutElement::GetColorAttr(LayoutAttr attr_type, int index)
{
    if(!m_pTaggedElement) {
        return 0;
    }
    CFX_ByteStringC owner = GetAttrOwner(attr_type);
    FX_COLORREF default_value = GetDefaultColorValue(attr_type);
    CFX_ByteStringC AttrName = ConvertLayoutAttr(attr_type);
    FX_ARGB f = m_pTaggedElement->GetColor(owner, AttrName, default_value, IsInheritable(attr_type), index);
    return f;
}
FX_FLOAT CPDF_LayoutElement::GetDefaultFloatValue(LayoutAttr attr_type)
{
    switch(attr_type) {
        case LayoutColumnCount:
            return 1;
        case LayoutRowSpan:
            return 1;
        case LayoutColSpan:
            return 1;
        default:
            return 0;
    }
}
FX_COLORREF CPDF_LayoutElement::GetDefaultColorValue(LayoutAttr attr_type)
{
    return -1;
}
CFX_ByteStringC CPDF_LayoutElement::GetDefaultNameValue(LayoutAttr attr_type)
{
    switch(attr_type) {
        case LayoutPlacement:
            return "Inline";
        case LayoutWritingMode:
            return "LrTb";
        case LayoutBorderStyle:
            return "None";
        case LayoutTextAlign:
            return "Start";
        case LayoutBlockAlign:
            return "Before";
        case LayoutInlineAlign:
            return "Start";
        case LayoutTBorderStyle:
            return "None";
        case LayoutTextDecorationType:
            return "None";
        case LayoutRubyAlign:
            return "Distribute";
        case LayoutRubyPosition:
            return "Before";
        case LayoutGlyphOrientationVertical:
            return "Auto";
        case LayoutListNumbering:
            return "None";
        case LayoutFieldRole:
            return "None";
        default:
            return "";
    }
}
FX_BOOL	CPDF_LayoutElement::IsInheritable(LayoutAttr type)
{
    switch(type) {
        case LayoutWritingMode:
        case LayoutTextAlign:
        case LayoutBlockAlign:
        case LayoutInlineAlign:
        case LayoutLineHeight:
        case LayoutGlyphOrientationVertical:
        case LayoutRubyAlign:
        case LayoutRubyPosition:
        case LayoutBorderThickness:
        case LayoutStartIndent:
        case LayoutEndIndent:
        case LayoutTextIndent:
        case LayoutTPadding:
        case LayoutTextDecorationThickness:
        case LayoutBorderColor:
        case LayoutColor:
        case LayoutTextDecorationColor:
            return TRUE;
        default:
            return FALSE;
    }
}
int	CPDF_LayoutElement::CountChildren()
{
    return m_ChildArray.GetSize();
}
IPDF_LayoutElement* CPDF_LayoutElement::GetChild(int index)
{
    return (IPDF_LayoutElement*)m_ChildArray.GetAt(index);
}
IPDF_LayoutElement* CPDF_LayoutElement::GetParent()
{
    return m_pParentElement;
}
int	CPDF_LayoutElement::CountObjects()
{
    if(m_pTaggedElement == NULL) {
        return 0;
    }
    CFX_PtrArray* pObj = &m_ObjArray;
    int size = pObj->GetSize();
    return size;
}
CPDF_PageObject* CPDF_LayoutElement::GetObject(int index)
{
    if(m_pTaggedElement == NULL) {
        return NULL;
    }
    CFX_PtrArray *pObj = &m_ObjArray;
    int size = pObj->GetSize();
    if(index < size) {
        return (CPDF_PageObject*)pObj->GetAt(index);
    }
    return NULL;
}
FX_BOOL CPDF_LayoutElement::AddObject(CPDF_PageObject* pObj)
{
    return m_ObjArray.Add(pObj);
}
IPDF_LayoutProvider* IPDF_LayoutProvider::Create_LayoutProvider_TaggedPDF(CPDF_PageObjects* pPage)
{
    if(pPage == NULL) {
        return NULL;
    }
    CPDF_LayoutProvider_TaggedPDF* pProvider = new CPDF_LayoutProvider_TaggedPDF;
    pProvider->Init(pPage);
    return pProvider;
}
CPDF_LayoutProvider_TaggedPDF::CPDF_LayoutProvider_TaggedPDF()
{
    m_pPause = NULL;
    m_pRoot = NULL;
    m_pPageTree = NULL;
    m_pCurTaggedElement = NULL;
}
CPDF_LayoutProvider_TaggedPDF::~CPDF_LayoutProvider_TaggedPDF()
{
    m_pCurTaggedElement = NULL;
    m_pPause = NULL;
    if(m_pRoot) {
        delete m_pRoot;
    }
    m_pRoot = NULL;
    if(m_pPageTree) {
        delete m_pPageTree;
    }
    m_pPageTree = NULL;
}
void CPDF_LayoutProvider_TaggedPDF::ProcessElement(CPDF_LayoutElement*pParent, CPDF_StructElement* pTaggedElement)
{
    if(!pTaggedElement) {
        return;
    }
    if(!pParent) {
        m_Status = LayoutError;
        return;
    }
    CPDF_LayoutElement* pElement = new CPDF_LayoutElement;
    pElement->m_pParentElement = pParent;
    pElement->m_pTaggedElement = pTaggedElement;
    pParent->m_ChildArray.Add(pElement);
    int count = pTaggedElement->CountKids();
    for(int i = 0; i < count; i++) {
        CPDF_StructKid Kid = pTaggedElement->GetKid(i);
        switch(Kid.m_Type) {
            case CPDF_StructKid::Element: {
                    ProcessElement(pElement, Kid.m_Element.m_pElement);
                    if(m_Status != LayoutReady) {
                        return ;
                    }
                }
                break;
            case CPDF_StructKid::PageContent: {
                    int count = m_pPage->CountObjects();
                    FX_POSITION pos = m_pPage->GetFirstObjectPosition();
                    if(!pos) {
                        m_Status = LayoutError;
                        return ;
                    }
                    while (pos) {
                        CPDF_PageObject* pObj = m_pPage->GetNextObject(pos);
                        int pbjMCID = pObj->m_ContentMark.GetMCID();
                        if((FX_DWORD)(pObj->m_ContentMark.GetMCID()) == Kid.m_PageContent.m_ContentId) {
                            pElement->AddObject(pObj);
                        }
                    }
                }
                break;
            case CPDF_StructKid::StreamContent:
            case CPDF_StructKid::Object:
            default:
                break;
        }
    }
}
LayoutStatus CPDF_LayoutProvider_TaggedPDF::StartLoad(IFX_Pause* pPause)
{
    m_pPause = pPause;
    if(m_pPage->m_pDocument && m_pPage->m_pFormDict) {
        m_pPageTree = CPDF_StructTree::LoadPage(m_pPage->m_pDocument, m_pPage->m_pFormDict);
    }
    if(!m_pPageTree) {
        m_Status = LayoutError;
        return LayoutError;
    }
    int count = m_pPageTree->CountTopElements();
    if(count == 0) {
        m_Status = LayoutError;
        return LayoutError;
    }
    m_pRoot = new CPDF_LayoutElement;
    for(int i = 0; i < count; i++) {
        CPDF_StructElement* pElement = m_pPageTree->GetTopElement(i);
        if(pElement) {
            ProcessElement(m_pRoot, pElement);
            if(m_Status != LayoutReady) {
                return m_Status;
            }
        }
    }
    m_pCurTaggedElement = NULL;
    m_Status = LayoutFinished;
    return LayoutFinished;
}
LayoutStatus CPDF_LayoutProvider_TaggedPDF::Continue()
{
    if(!m_pCurTaggedElement) {
        return LayoutError;
    }
    if(m_Status != LayoutToBeContinued) {
        return LayoutError;
    }
    m_Status = LayoutReady;
    int count = m_pPageTree->CountTopElements();
    for(int i = 0; i < count; i++) {
        CPDF_StructElement* pElement = m_pPageTree->GetTopElement(i);
        if(pElement) {
            ProcessElement(m_pRoot, pElement);
            if(m_Status != LayoutReady) {
                return m_Status;
            }
        }
    }
    m_pCurTaggedElement = NULL;
    m_Status = LayoutFinished;
    return LayoutFinished;
}
int CPDF_LayoutProvider_TaggedPDF::GetPosition()
{
    if(m_TopElementIndex == 0) {
        return 0;
    }
    int count = m_pPageTree->CountTopElements();
    return m_TopElementIndex / count * 100;
}
