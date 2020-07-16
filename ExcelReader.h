#pragma once

#include <Ole2.h>

class ExcelReader
{
private:
    IDispatch *m_pXlApp;
    IDispatch *m_pXlBooks;
    IDispatch *m_pXlBook;
    IDispatch *m_pXlSheets;
    IDispatch *m_pXlSheet;

    HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPCOLESTR ptName, int cArgs...);
public:
    ExcelReader()
    {
        m_pXlApp = nullptr;
        m_pXlBooks = nullptr;
        m_pXlBook = nullptr;
        m_pXlSheets = nullptr;
        m_pXlSheet = nullptr;
    }
    ~ExcelReader()
    {
        m_pXlSheet->Release();
        m_pXlSheets->Release();
        m_pXlBook->Release();
        m_pXlBooks->Release();
        m_pXlApp->Release();
    }
    int Start();
    int RowsColsCount(int *nRows, int *nCols);
    void Print();
};
