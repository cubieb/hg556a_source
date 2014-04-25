
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define NAME_SIZE  64
#define LIST_STEP  4

typedef struct {
	char szName[NAME_SIZE];
} NAME_S;

/* 1 -white, 0 -black. */
static unsigned int s_uMode = 0;
static NAME_S* s_pstNameList = 0;
static unsigned int s_uNameSize = 0;


unsigned int filterInitialize(const char* lpszFile)
{
	FILE* pFile = 0;
	char* pszTemp = 0;
	NAME_S* pstName = 0;
	unsigned int uInd = 0;
	char szName[NAME_SIZE] = {0};

	if (s_pstNameList != 0) {
		printf("URL Filer: Has been initialized!\n");
		return 0;
	}
	
	pFile = fopen(lpszFile, "r");
	if (pFile == 0) {
		printf("URL Filer: open rule file failed!\n");
		return 1;
	}

    /* get white/black mode */
	if (!fgets(szName, sizeof(szName), pFile)) {
		printf("URL Filer: Not sepcial white/black mode in file!\n");
		fclose(pFile);
		return 1;
	}
	s_uMode = (unsigned int)atoi(szName);

	/* get name list */
	while (fgets(szName, sizeof(szName), pFile)) {
		if (s_uNameSize <= uInd) {
			pstName = realloc(s_pstNameList, 
				(sizeof(NAME_S) * (s_uNameSize + LIST_STEP)));
			if (pstName == 0) {
				if (s_pstNameList != 0) {
					free(s_pstNameList);
					s_pstNameList = 0;
				}
				uInd = 0;
				break;
			}
			memset(&(pstName[uInd]), 0, (sizeof(NAME_S) * LIST_STEP));
			s_pstNameList = pstName;
			s_uNameSize += LIST_STEP;
		}
		if (szName[0] == '\n'
			|| szName[0] == '\r'
			|| szName[0] == ' '
			|| szName[0] == '\t')
		{
			continue;
		}
		
		pszTemp = strpbrk(szName, " \r\n");
		if (pszTemp != 0) {
			*pszTemp = '\0';
		}
		strncpy(s_pstNameList[uInd].szName, szName, NAME_SIZE);
		s_pstNameList[uInd].szName[NAME_SIZE - sizeof(char)] = '\0';
		
		uInd++;
	}

	fclose(pFile);
	s_uNameSize = uInd;
	return 1;
}

void filterClean()
{
	s_uMode = 0;
	if (s_pstNameList != 0) {
		free(s_pstNameList);
		s_pstNameList = 0;
		s_uNameSize = 0;
	}
}

unsigned int filterCheck(const char* lpszName)
{
	
	if (s_pstNameList != 0) {
		unsigned int uInd = 0;
		for (uInd = 0; uInd < s_uNameSize; uInd++) {
			if (strcasestr(lpszName, s_pstNameList[uInd].szName) != 0) {
				return s_uMode;
			}
		}
	}
	return (!s_uMode);
}
