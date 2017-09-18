#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pf.h>
#include <pf_internal.h>

#define MAX_LENGTH 32

typedef struct record{
	char name[MAX_LENGTH];
	char addr[MAX_LENGTH * 2];
	char telnum[MAX_LENGTH];
	char email[MAX_LENGTH];
}RecordT;

//function prototype define 
int menuDisplay();
int insertContact(PF_FileHandle &fh);
int deleteContact(PF_FileHandle &fh);
int updateContact(PF_FileHandle &fh);
int searchContact(PF_FileHandle &fh);
int showAllContact(PF_FileHandle &fh);

int main(){
	PF_Manager pfm;
	PF_FileHandle fh;
	PF_PageHandle ph;
	RC rc;
	int select;

	if (rc = pfm.CreateFile("Contactlist.dat"))
	{
		printf("\nDataFile is already exist!\n");
	}
	if((rc = pfm.OpenFile("Contactlist.dat", fh)))
	{
		printf("DataFile open error!\n");
		return (rc);
	}
	
	system("clear");
	while((select = menuDisplay()) != 6){
		switch(select){
			case 1 :
				insertContact(fh);
				break;
			case 2 :
				deleteContact(fh);
				break;
			case 3 :
				updateContact(fh);
				break;
			case 4 :
				searchContact(fh);
				break;
			case 5 :
				showAllContact(fh);
				break;
			default :
				break;
		}
	}

	pfm.CloseFile(fh);

	return 0;
}

// Print the menu screen
int menuDisplay(){
	int select;

	system("clear");
	printf("\n  CONTACT MANAGER\n");
	printf("=====================\n");
	printf("    1. Insert\n");
	printf("    2. Delete\n");
	printf("    3. Update\n");
	printf("    4. Search\n");
	printf("    5. Show All\n");
	printf("    6. Exit\n");
	printf("=====================\n");
	printf("Press Menu Number>>");
	
	select = getchar() - 48;
	system("clear");

	return select;
}

// Option !

void recordTo_pData(RecordT record, char * pData)
{
	sprintf(pData, "%s,%s,%s,%s",record.name, record.addr, record.telnum, record.email);
}

RecordT pDataToRecord(char * pData) {
	RecordT person;
	char * temp = (char *)malloc(4096);
	strncpy(temp, pData, 4096);
	strcpy(person.name, strtok(temp, ","));
	strcpy(person.addr, strtok(NULL, ","));
	strcpy(person.telnum, strtok(NULL, ","));
	strcpy(person.email, strtok(NULL, ","));
	free(temp);
	return person;
}

/*int findPage(char * name, PF_FileHandle &fh, PF_PageHandle &ph, char* &pData) {
	PageNum pageNum;
	RC rc;
	RecordT person;
	int PN = 0;
	char * temp = (char*)malloc(4096);

	if(rc = fh.GetFirstPage(ph))
	return (rc);

	do
	{
		if((rc = ph.GetData(pData)) || (rc = ph.GetPageNum(pageNum)))
		return (rc);

		strncpy(temp, pData, 4096);
		person = pDataToRecord(temp);

		if(!strcmp(name, person.name))
		{
			ph.GetPageNum(pageNum);
			PN = pageNum;
			//printf("%d\n",PN);
			fh.UnpinPage(pageNum);
			return PN;
		}
		fh.UnpinPage(pageNum);
	}while(!fh.GetNextPage(pageNum,ph));

	free(temp);

	return -1;
}*/
int findPage(char * name, PF_FileHandle &fh, PF_PageHandle &ph, char* &pData)
{
	PageNum pageNum;
	RC rc;
	RecordT person;
	char * old = (char *)malloc(4096);
	if((rc = fh.GetFirstPage(ph)))
	return (rc);

	do {
		if((rc = ph.GetData(pData)) || (rc = ph.GetPageNum(pageNum)))
	return (rc);

	strncpy(old, pData, 4096);
	person = pDataToRecord(old);

	if(strcmp(person.name, name) == 0)
	{
		free(old);
		fh.UnpinPage(pageNum);
		return pageNum;
	}
	if (rc = fh.UnpinPage(pageNum))
	return (rc);
	} while (!(rc = fh.GetNextPage(pageNum, ph)));
	free(old);
	return -1;
}
// Set basic information before performing the operation
// Insert the item in a record unit
int insertContact(PF_FileHandle &fh) {
	RC rc;
	char * pData;
	PageNum pageNum;
	PF_PageHandle ph;
	RecordT person;

	//system("clear");
	printf("name : ");
	scanf("%s", person.name);
	printf("addr : ");
	scanf("%s", person.addr);
	printf("telnum : ");
	scanf("%s", person.telnum);
	printf("email : ");
	scanf("%s", person.email);

	if((rc = fh.AllocatePage(ph)) ||
	 (rc = ph.GetData(pData)) || (rc = ph.GetPageNum(pageNum)))
	return (rc);

	recordTo_pData(person, pData);
	printf("Contact INSERT COMPLETE !\n");

	fh.MarkDirty(pageNum);
	fh.UnpinPage(pageNum);

	printf("\nPress Enter key to go back menu screen\n");
	getchar();
	getchar();

	return 0;
}

// Delete the item in a record unit by the appropriate field
int deleteContact(PF_FileHandle &fh) {
        PF_PageHandle ph;
        RC rc;
        PageNum pageNum;
        char * pData;
        char person_name[32];

        printf("Input a name that you want to DELETE : ");
		scanf("%s",person_name);
        
		pageNum = findPage(person_name, fh, ph, pData);
        if (pageNum == -1) {
                printf("\nYour Information doesn't exist\n");
                return 0;
        }
        else {
                printf("Contact DELETED !\n");

        }
        if (rc = fh.UnpinPage(pageNum))
                return (rc);
        if (rc = fh.DisposePage(pageNum))
                return (rc);

        printf("\nPress Enter key to go back menu screen\n");
        getchar();
        getchar();

        return 0;
}

// Update the item in a record unit by the appropriate field
int updateContact(PF_FileHandle &fh) {
	RC rc;
	PageNum pageNum;
	RecordT person;
	PF_PageHandle ph;
	char * pData;
	char person_name[32];

	printf(" Enter the information you want to search for UPDATE! >> ");
	scanf("%s", person_name);
	pageNum = findPage(person_name, fh, ph, pData);

		if (pageNum == -1) {
			printf("\nYour Information doesn't exist\n");
			return 0;
		}
		else {
			strcpy(person.name, person_name);
			printf("NEW addr : ");
			scanf("%s", person.addr);
			printf("NEW telnum : ");
			scanf("%s", person.telnum);
			printf("NEW email : ");
			scanf("%s", person.email);

			printf("UPDATE COMPLETE ! \n");

			if ((rc = ph.GetData(pData)))
				return (rc);
			recordTo_pData(person, pData);
		}
		if (rc = fh.UnpinPage(pageNum))
			return (rc);

		printf("\nPress Enter key to go back menu screen\n");
		getchar();
		getchar();

		return 0;
}
// Search the item in a record unit by the appropriate field
int searchContact(PF_FileHandle &fh) {
	PF_PageHandle ph;
	char person_name[32];
	RecordT person;
	char * pData;
	PageNum pageNum;

	printf("Input a name that you want to SEARCH : ");
	scanf("%s",person_name); //Save in buff Array

	pageNum = findPage(person_name, fh, ph, pData);

	if (pageNum == -1) {
		printf("\nYour Information doesn't exist...\n");
		return -1;
	}

	person = pDataToRecord(pData);
	
	printf("\n     The result you want\n");
	printf("=============================\n");
	printf("name    : %s\n",person.name);
	printf("address : %s\n",person.addr);
	printf("contact : %s\n",person.telnum);
	printf("email   : %s\n",person.email);
	printf("=============================\n");

	printf("\nPress Enter key to go back menu screen\n");
	getchar();
	getchar();

	return 0;
}

// Print all Items existing in file
int showAllContact(PF_FileHandle &fh){
	PF_PageHandle ph;
	RecordT person;
	char * pData;
	PageNum pageNum;
	RC rc;
	char * temp = (char*)malloc(4096);

	fh.GetFirstPage(ph);

	do {
		ph.GetData(pData);
		ph.GetPageNum(pageNum);
		strncpy(temp, pData, 4096);
		person = pDataToRecord(temp);

		printf("\n     The result you want\n");
		printf("=============================\n");
		printf("name    : %s\n",person.name);
		printf("address : %s\n",person.addr);
		printf("contact : %s\n",person.telnum);
		printf("email   : %s\n",person.email);
		printf("=============================\n");
	} while(!(rc = fh.GetNextPage(pageNum, ph)));
	free(temp);
	fh.UnpinPage(pageNum);

	printf("\nPress Enter key to go back menu screen\n");
	getchar();
	getchar();

	return 0;
}


