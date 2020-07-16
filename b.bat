..\..\externalTools\windows_bison_flex\win_bison.exe -x -v -d parserCmd.y -o parserCmd.tab.cpp
..\..\externalTools\windows_bison_flex\\win_flex.exe --outfile=scannerCmd.yy.cpp --header-file=scannerCmd.yy.h --nounistd scannerCmd.l 
