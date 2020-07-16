#include "ExcelReader.h"
#include <iostream>

void ExcelReader::Print()
{
    VARIANT resultCell;

    for (int row = 1; row <= 10; row++)
        for (int col = 1; col <= 10; col++)
        {
            VariantInit(&resultCell);

            VARIANT parmRow, paramCol;
            parmRow.vt = VT_R8;
            parmRow.dblVal = row;

            paramCol.vt = VT_R8;
            paramCol.dblVal = col;

            AutoWrap(DISPATCH_PROPERTYGET, &resultCell, m_pXlSheet, L"Cells", 2, paramCol, parmRow);
            IDispatch *pCell = resultCell.pdispVal;
            AutoWrap(DISPATCH_PROPERTYGET, &resultCell, pCell, L"Value", 0);

            VARTYPE Type = resultCell.vt;
            WCHAR text[4096];
            int ret;
            text[0] = 0;
            switch (Type)
            {
            case VT_UI1:
            {
                unsigned char nChr = resultCell.bVal;
                ret = wsprintf(text, L"%d", nChr);
            }
            break;
            case VT_I4:
            {
                long nVal = resultCell.lVal;
                ret = wsprintf(text, L"%i", nVal);
            }
            break;
            case VT_R4:
            {
                float fVal = resultCell.fltVal;
                ret = wsprintf(text, L"%f", fVal);
            }
            break;
            case VT_R8:
            {
                double dVal = resultCell.dblVal;
                ret = wsprintf(text, L"%d", (int)dVal);
            }
            break;
            case VT_BSTR:
            {
                BSTR b = resultCell.bstrVal;
                ret = wsprintf(text, L"%s", b);
            }
            break;
            case VT_BYREF | VT_UI1:
            {
                //Not tested
                unsigned char* pChr = resultCell.pbVal;
                ret = wsprintf(text, L"%d", pChr);
            }
            break;
            case VT_BYREF | VT_BSTR:
            {
                //Not tested
                BSTR* pb = resultCell.pbstrVal;
                ret = wsprintf(text, L"%s", *pb);
            }
            case 0:
            {
                //Empty
                //szValue = _T("");
            }

            break;
            }

            wprintf(L"%d, %d : %s\n", col, row, text);
        }
}

int ExcelReader::RowsColsCount(int *nRows, int *nCols)
{
    VARIANT result2;

    VariantInit(&result2);

    VARIANT result3;
    VariantInit(&result3);
    AutoWrap(DISPATCH_PROPERTYGET, &result2, m_pXlSheet, L"UsedRange", 0);
    IDispatch *pXlRangeUsed = result2.pdispVal;

    AutoWrap(DISPATCH_PROPERTYGET, &result3, pXlRangeUsed, L"Columns", 0);

    IDispatch *pXlRangeCols = result3.pdispVal;


    VARIANT result4;
    VariantInit(&result4);
    AutoWrap(DISPATCH_PROPERTYGET, &result4, pXlRangeCols, L"Count", 0);
    *nCols = result4.intVal;


    AutoWrap(DISPATCH_PROPERTYGET, &result3, pXlRangeUsed, L"Rows", 0);
    IDispatch *pXlRangeRows = result3.pdispVal;
    AutoWrap(DISPATCH_PROPERTYGET, &result4, pXlRangeRows, L"Count", 0);
    *nRows = result4.intVal;

    pXlRangeCols->Release();
    pXlRangeRows->Release();
    pXlRangeUsed->Release();

    return -1;
}

int ExcelReader::Start()
{
    // Initialize COM for this thread...
    CoInitialize(NULL);

    // Get CLSID for our server...
    CLSID clsid;
    HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

    if (FAILED(hr)) {

        ::MessageBox(NULL, L"CLSIDFromProgID() failed", L"Error", 0x10010);
        return -1;
    }

    hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **)&m_pXlApp);
    if (FAILED(hr)) {
        ::MessageBox(NULL, L"Excel not registered properly", L"Error", 0x10010);
        return -2;
    }

    // Make it visible (i.e. app.visible = 1)
    {

        VARIANT x;
        x.vt = VT_I4;
        x.lVal = 1;
        AutoWrap(DISPATCH_PROPERTYPUT, NULL, m_pXlApp, L"Visible", 1, x);
    }

    {
        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_PROPERTYGET, &result, m_pXlApp, L"Workbooks", 0);
        m_pXlBooks = result.pdispVal;
    }

    {
        VARIANT x, x2;
        VariantInit(&x);
        VariantInit(&x2);
        x.vt = VT_BSTR;
        x.bstrVal = SysAllocString(L"Filename");
        x2.vt = VT_BSTR;
        x2.bstrVal = SysAllocString(L"c:\\temp\\TM_PHEV_SRTS_A.00.50_r.xlsx");

        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_PROPERTYGET, &result, m_pXlBooks, L"Open", 1, x2);
        m_pXlBook = result.pdispVal;
    }


    {
        VARIANT result;
        VariantInit(&result);
        //AutoWrap(DISPATCH_PROPERTYGET, &result, m_pXlApp, L"ActiveSheet", 0);
        AutoWrap(DISPATCH_PROPERTYGET, &result, m_pXlBook, L"Worksheets", 0);
        m_pXlSheets = result.pdispVal;


        VARIANT result4;
        VariantInit(&result4);
        AutoWrap(DISPATCH_PROPERTYGET, &result4, m_pXlSheets, L"Count", 0);
        int xount = result4.intVal;
        {
            xount++;
        }
        VariantInit(&result);
        VARIANT x;
        x.vt = VT_I4;
        x.lVal = 2;
        AutoWrap(DISPATCH_PROPERTYGET, &result, m_pXlSheets, L"Item", 1, x);

        m_pXlSheet = result.pdispVal;
    }
}

void ShowException(LPCOLESTR szMember, HRESULT hr, EXCEPINFO *pexcep, unsigned int uiArgErr)
{
    TCHAR szBuf[512];

    switch (GetScode(hr))
    {
    case DISP_E_UNKNOWNNAME:
        wsprintf(szBuf, L"%s: Unknown name or named argument.", szMember);
        break;

    case DISP_E_BADPARAMCOUNT:
        wsprintf(szBuf, L"%s: Incorrect number of arguments.", szMember);
        break;

    case DISP_E_EXCEPTION:
        wsprintf(szBuf, L"%s: Error %d: ", szMember, pexcep->wCode);
        if (pexcep->bstrDescription != NULL)
            lstrcat(szBuf, (WCHAR*)pexcep->bstrDescription);
        else
            lstrcat(szBuf, L"<<No Description>>");
        break;

    case DISP_E_MEMBERNOTFOUND:
        wsprintf(szBuf, L"%s: method or property not found.", szMember);
        break;

    case DISP_E_OVERFLOW:
        wsprintf(szBuf, L"%s: Overflow while coercing argument values.", szMember);
        break;

    case DISP_E_NONAMEDARGS:
        wsprintf(szBuf, L"%s: Object implementation does not support named arguments.",
            szMember);
        break;

    case DISP_E_UNKNOWNLCID:
        wsprintf(szBuf, L"%s: The locale ID is unknown.", szMember);
        break;

    case DISP_E_PARAMNOTOPTIONAL:
        wsprintf(szBuf, L"%s: Missing a required parameter.", szMember);
        break;

    case DISP_E_PARAMNOTFOUND:
        wsprintf(szBuf, L"%s: Argument not found, argument %d.", szMember, uiArgErr);
        break;

    case DISP_E_TYPEMISMATCH:
        wsprintf(szBuf, L"%s: Type mismatch, argument %d.", szMember, uiArgErr);
        break;

    default:
        wsprintf(szBuf, L"%s: Unknown error occured.", szMember);
        break;
    }

    MessageBox(NULL, szBuf, L"OLE Error", MB_OK | MB_ICONSTOP);

}

// AutoWrap() - Automation helper function...
HRESULT ExcelReader::AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPCOLESTR ptName, int cArgs...) {
    // Begin variable-argument list...
    va_list marker;
    va_start(marker, cArgs);

    if (!pDisp) {
        MessageBox(NULL, L"NULL IDispatch passed to AutoWrap()", L"Error", 0x10010);
        _exit(0);
    }

    // Variables used...
    DISPPARAMS dp = { NULL, NULL, 0, 0 };
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;
    HRESULT hr;
    TCHAR buf[200];
    //TCHAR szName[200];


    // Convert down to ANSI
    //WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);

    // Get DISPID for name passed...
    hr = pDisp->GetIDsOfNames(IID_NULL, (LPOLESTR*) &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
    if (FAILED(hr)) {
        swprintf(buf, L"IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx", ptName, hr);
        MessageBox(NULL, buf, L"AutoWrap()", 0x10010);
        _exit(0);
        return hr;
    }

    // Allocate memory for arguments...
    VARIANT *pArgs = new VARIANT[cArgs + 1];
    // Extract arguments...
    for (int i = 0; i < cArgs; i++) {
        pArgs[i] = va_arg(marker, VARIANT);
    }

    // Build DISPPARAMS
    dp.cArgs = cArgs;
    dp.rgvarg = pArgs;

    // Handle special-case for property-puts!
    if (autoType & DISPATCH_PROPERTYPUT) {
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &dispidNamed;
    }

    // Make the call!
    hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL);
    if (FAILED(hr)) {
        ShowException(ptName, hr, NULL, 0);
        swprintf(buf, L"IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx", ptName, dispID, hr);
        MessageBox(NULL, buf, L"AutoWrap()", 0x10010);
        _exit(0);
        return hr;
    }
    // End variable-argument section...
    va_end(marker);

    delete[] pArgs;

    return hr;
}
