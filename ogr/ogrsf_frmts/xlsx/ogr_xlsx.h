/******************************************************************************
 *
 * Project:  XLSX Translator
 * Purpose:  Definition of classes for OGR OpenOfficeSpreadsheet .xlsx driver.
 * Author:   Even Rouault, even dot rouault at spatialys.com
 *
 ******************************************************************************
 * Copyright (c) 2012, Even Rouault <even dot rouault at spatialys.com>
 *
 * SPDX-License-Identifier: MIT
 ****************************************************************************/

#ifndef OGR_XLSX_H_INCLUDED
#define OGR_XLSX_H_INCLUDED

#include "ogrsf_frmts.h"

#include "ogr_expat.h"
#include "memdataset.h"

#include <vector>
#include <set>
#include <string>
#include <map>

namespace OGRXLSX
{

/************************************************************************/
/*                             OGRXLSXLayer                             */
/************************************************************************/

class OGRXLSXDataSource;

class OGRXLSXLayer final : public OGRMemLayer
{
    friend class OGRXLSXDataSource;

    bool bInit;
    OGRXLSXDataSource *poDS;
    CPLString osFilename;
    void Init();
    bool bUpdated;
    bool bHasHeaderLine;
    std::string m_osCols{};
    std::set<int> oSetFieldsOfUnknownType{};

    GIntBig TranslateFIDFromMemLayer(GIntBig nFID) const;
    GIntBig TranslateFIDToMemLayer(GIntBig nFID) const;

  public:
    OGRXLSXLayer(OGRXLSXDataSource *poDSIn, const char *pszFilename,
                 const char *pszName, int bUpdateIn = FALSE);

    bool HasBeenUpdated() const
    {
        return bUpdated;
    }

    void SetUpdated(bool bUpdatedIn = true);

    bool GetHasHeaderLine() const
    {
        return bHasHeaderLine;
    }

    void SetHasHeaderLine(bool bIn)
    {
        bHasHeaderLine = bIn;
    }

    const char *GetName() override
    {
        return OGRMemLayer::GetLayerDefn()->GetName();
    }

    OGRwkbGeometryType GetGeomType() override
    {
        return wkbNone;
    }

    virtual OGRSpatialReference *GetSpatialRef() override
    {
        return nullptr;
    }

    void ResetReading() override
    {
        Init();
        OGRMemLayer::ResetReading();
    }

    const CPLString &GetFilename() const
    {
        return osFilename;
    }

    /* For external usage. Mess with FID */
    virtual OGRFeature *GetNextFeature() override;
    virtual OGRFeature *GetFeature(GIntBig nFeatureId) override;
    virtual OGRErr ISetFeature(OGRFeature *poFeature) override;
    OGRErr IUpdateFeature(OGRFeature *poFeature, int nUpdatedFieldsCount,
                          const int *panUpdatedFieldsIdx,
                          int nUpdatedGeomFieldsCount,
                          const int *panUpdatedGeomFieldsIdx,
                          bool bUpdateStyleString) override;
    virtual OGRErr DeleteFeature(GIntBig nFID) override;

    virtual OGRErr SetNextByIndex(GIntBig nIndex) override
    {
        Init();
        return OGRMemLayer::SetNextByIndex(nIndex);
    }

    virtual OGRErr ICreateFeature(OGRFeature *poFeature) override;

    OGRFeatureDefn *GetLayerDefn() override
    {
        Init();
        return OGRMemLayer::GetLayerDefn();
    }

    GIntBig GetFeatureCount(int bForce) override
    {
        Init();
        return OGRMemLayer::GetFeatureCount(bForce);
    }

    virtual OGRErr CreateField(const OGRFieldDefn *poField,
                               int bApproxOK = TRUE) override;

    virtual OGRErr DeleteField(int iField) override
    {
        Init();
        SetUpdated();
        return OGRMemLayer::DeleteField(iField);
    }

    virtual OGRErr ReorderFields(int *panMap) override
    {
        Init();
        SetUpdated();
        return OGRMemLayer::ReorderFields(panMap);
    }

    virtual OGRErr AlterFieldDefn(int iField, OGRFieldDefn *poNewFieldDefn,
                                  int nFlagsIn) override
    {
        Init();
        SetUpdated();
        return OGRMemLayer::AlterFieldDefn(iField, poNewFieldDefn, nFlagsIn);
    }

    int TestCapability(const char *pszCap) override
    {
        Init();
        return OGRMemLayer::TestCapability(pszCap);
    }

    const std::string &GetCols() const
    {
        return m_osCols;
    }

    virtual OGRErr SyncToDisk() override;

    GDALDataset *GetDataset() override;
};

/************************************************************************/
/*                           OGRXLSXDataSource                          */
/************************************************************************/
#define STACK_SIZE 5

typedef enum
{
    STATE_DEFAULT,

    /* for sharedString.xml */
    STATE_SI,
    STATE_T,

    /* for sheet?.xml */
    STATE_COLS,
    STATE_SHEETDATA,
    STATE_ROW,
    STATE_CELL,
    STATE_TEXTV,
} HandlerStateEnum;

typedef struct
{
    HandlerStateEnum eVal;
    int nBeginDepth;
} HandlerState;

class XLSXFieldTypeExtended
{
  public:
    OGRFieldType eType;
    bool bHasMS;

    XLSXFieldTypeExtended() : eType(OFTMaxType), bHasMS(false)
    {
    }

    explicit XLSXFieldTypeExtended(OGRFieldType eTypeIn, bool bHasMSIn = false)
        : eType(eTypeIn), bHasMS(bHasMSIn)
    {
    }
};

class OGRXLSXDataSource final : public GDALDataset
{
    char *pszName;
    CPLString osPrefixedFilename;
    bool bUpdatable;
    bool bUpdated;

    int nLayers;
    OGRXLSXLayer **papoLayers;
    std::map<CPLString, CPLString> oMapRelsIdToTarget;
    std::set<std::string> m_oSetSheetId;

    void AnalyseSharedStrings(VSILFILE *fpSharedStrings);
    void AnalyseWorkbook(VSILFILE *fpWorkbook);
    void AnalyseWorkbookRels(VSILFILE *fpWorkbookRels);
    void AnalyseStyles(VSILFILE *fpStyles);

    std::vector<std::string> apoSharedStrings;
    std::string osCurrentString;

    bool bFirstLineIsHeaders;
    int bAutodetectTypes;

    XML_Parser oParser;
    bool bStopParsing;
    int nWithoutEventCounter;
    int nDataHandlerCounter;
    int nCurLine;
    int nCurCol;

    OGRXLSXLayer *poCurLayer;
    std::string m_osCols{};

    int nStackDepth;
    int nDepth;
    HandlerState stateStack[STACK_SIZE];

    CPLString osValueType;
    CPLString osValue;

    std::vector<std::string> apoFirstLineValues;
    std::vector<std::string> apoFirstLineTypes;
    std::vector<std::string> apoCurLineValues;
    std::vector<std::string> apoCurLineTypes;

    bool bInCellXFS;
    std::map<int, XLSXFieldTypeExtended> apoMapStyleFormats;
    std::vector<XLSXFieldTypeExtended> apoStyles;

    void PushState(HandlerStateEnum eVal);
    void startElementDefault(const char *pszName, const char **ppszAttr);
    void startElementTable(const char *pszName, const char **ppszAttr);
    void endElementTable(const char *pszName);
    void startElementCols(const char *pszName, const char **ppszAttr);
    void endElementCols(const char *pszName);
    void startElementRow(const char *pszName, const char **ppszAttr);
    void endElementRow(const char *pszName);
    void startElementCell(const char *pszName, const char **ppszAttr);
    void endElementCell(const char *pszName);
    void dataHandlerTextV(const char *data, int nLen);

    void DetectHeaderLine();

    OGRFieldType GetOGRFieldType(const char *pszValue, const char *pszValueType,
                                 OGRFieldSubType &eSubType);

    void DeleteLayer(const char *pszLayerName);

  public:
    explicit OGRXLSXDataSource(CSLConstList papszOpenOptionsIn);
    virtual ~OGRXLSXDataSource();
    CPLErr Close() override;

    int Open(const char *pszFilename, const char *pszPrefixedFilename,
             VSILFILE *fpWorkbook, VSILFILE *fpWorkbookRels,
             VSILFILE *fpSharedStrings, VSILFILE *fpStyles, int bUpdate);
    int Create(const char *pszName, char **papszOptions);

    virtual int GetLayerCount() override;
    virtual OGRLayer *GetLayer(int) override;

    virtual int TestCapability(const char *) override;

    OGRLayer *ICreateLayer(const char *pszName,
                           const OGRGeomFieldDefn *poGeomFieldDefn,
                           CSLConstList papszOptions) override;

    virtual OGRErr DeleteLayer(int iLayer) override;

    virtual CPLErr FlushCache(bool bAtClosing) override;

    void startElementCbk(const char *pszName, const char **ppszAttr);
    void endElementCbk(const char *pszName);
    void dataHandlerCbk(const char *data, int nLen);

    void startElementSSCbk(const char *pszName, const char **ppszAttr);
    void endElementSSCbk(const char *pszName);
    void dataHandlerSSCbk(const char *data, int nLen);

    void startElementWBRelsCbk(const char *pszName, const char **ppszAttr);

    void startElementWBCbk(const char *pszName, const char **ppszAttr);

    void startElementStylesCbk(const char *pszName, const char **ppszAttr);
    void endElementStylesCbk(const char *pszName);

    void BuildLayer(OGRXLSXLayer *poLayer);

    bool GetUpdatable()
    {
        return bUpdatable;
    }

    void SetUpdated()
    {
        bUpdated = true;
    }
};

}  // namespace OGRXLSX

#endif /* ndef OGR_XLSX_H_INCLUDED */
