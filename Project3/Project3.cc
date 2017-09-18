#include <cstring>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "redbase.h"
#include "pf.h"
#include "rm.h"

using namespace std;
//
// Computes the offset of a field in a record (should be in <stddef.h>)
//
#ifndef offsetof
#       define offsetof(type, field)   ((size_t)&(((type *)0) -> field))
#endif

#define recordSize 160

struct PersonRecord {
	char *name;
	char *addr;
	char *telnum;
	char *email;
};

int menuDisplay();
int insertContact(RM_FileHandle &fh);
int insert40Contacts(RM_FileHandle &fh);
int othersContact(RM_FileHandle &fh, int val);
//int sortContact(RM_FileHandle &fh); // Sorted File
void PrintRMRecord(RM_Record &rec);

void StructInit(struct PersonRecord *person, int nsize, int asize, int tsize, int esize)
{
	person->name = (char*)malloc(sizeof(char) * nsize);
	person->addr = (char*)malloc(sizeof(char) * asize);
	person->telnum = (char*)malloc(sizeof(char) * tsize);
	person->email = (char*)malloc(sizeof(char) * esize);
}

int main() {
	PF_Manager pfm;
	RM_Manager rmm(pfm);
	RM_FileHandle fh;
	RC rc;

	int select;
	if ((rc = rmm.CreateFile("Contactlist_RM.dat", recordSize)))
		printf("DataFile is already exist!\n");
	else
		printf("We just made a datafile 'Contactlist2.dat'\n");

	if ((rc = rmm.OpenFile("Contactlist_RM.dat", fh))) {
		printf("DataFile open error!\n");
		return (rc);
	}
	else
		printf("DataFile open success!\n");



	system("clear");
	while ((select = menuDisplay()) != 7) {
		while (1) {
			if (getchar() == '\n')
				break;
		}
		switch (select) {
		case 1: // insert
			insertContact(fh);
			getchar();
			break;
		case 2: // delete
			othersContact(fh, 2);
			getchar();
			break;
		case 3: // update
			othersContact(fh, 3);
			getchar();
			break;
		case 4: // search
			othersContact(fh, 0);
			getchar();
			break;
		case 5: // show all
			othersContact(fh, 1);
			getchar();
			break;
		case 6: // insert 40 records
			insert40Contacts(fh);
			getchar();
			break;
		/*case 7: // SortContact
			sortContact(fh);
			getchar();
			break;*/
		default:
			break;
		}
	}
	rmm.CloseFile(fh);


	return 0;
}

int menuDisplay() {
	int select;

	system("clear");
	printf("\n  CONTACT MANAGER\n");
	printf("=====================\n");
	printf("    1. Insert\n");
	printf("    2. Delete\n");
	printf("    3. Update\n");
	printf("    4. Search\n");
	printf("    5. Show All\n");
	printf("    6. Insert 40 Records\n");
	printf("    7. SortList\n");
	printf("    8. Exit\n");
	printf("=====================\n");
	printf("Press Menu Number>>");

	select = getchar() - 48;
	system("clear");

	return select;
}

PersonRecord VlengthRecord() {
	PersonRecord person;
	char * vbuff = (char *)malloc(160);
	char * vname = (char *)malloc(32);
	char * vaddr = (char *)malloc(64);
	char * vtelnum = (char *)malloc(32);
	char * vemail = (char *)malloc(32);

	printf("Input your name that you want to put in : ");
	scanf("%31s", vname);
	printf("Input an address : ");
	scanf("%63s", vaddr);
	printf("Input a telnum : ");
	scanf("%31s", vtelnum);
	printf("Input an email : ");
	scanf("%31s", vemail);

	sprintf(vbuff, "%s,%s,%s,%s",vname,vaddr,vtelnum,vemail);

	StructInit(&person,sizeof(strtok(vbuff, ",")),sizeof(strtok(NULL, ",")),sizeof(strtok(NULL, ",")),sizeof(strtok(NULL, ",")));
	strcpy(person.name, strtok(vbuff, ","));
	strcpy(person.addr, strtok(NULL, ","));
	strcpy(person.telnum, strtok(NULL, ","));
	strcpy(person.email, strtok(NULL, ","));

	free(vname);
	free(vaddr);
	free(vtelnum);
	free(vemail);
	free(vbuff);

	return person;
}

int insertContact(RM_FileHandle &fh) {
	char *record;
	RID rid;
	PersonRecord person;

	memset(&person, 0, sizeof(person));

	person = VlengthRecord();
	record = (char *)&person;

	fh.InsertRec(record, rid);
	
	return 0;
}


int insert40Contacts(RM_FileHandle &fh) {
	char *record;
	RID rid;
	PersonRecord person;
	int i = 0;
	char testc[] = "test%d";

	printf("insert 40 Records!!\n");
	for (i = 0; i < 40; i++) {
		memset(&person, 0, sizeof(person));
		StructInit(&person,sizeof(testc),sizeof(testc),sizeof(testc),sizeof(testc));
		sprintf(person.name, testc, i);
		sprintf(person.addr, testc, i);
		sprintf(person.telnum, testc, i);
		sprintf(person.email, testc, i);

		record = (char *)&person;

		fh.InsertRec(record, rid);
	}
	return 0;
}


int othersContact(RM_FileHandle &fh, int val) {
	RC        rc;
	int       n;
	RID       rid;
	RM_Record rec;
	char person_name[32];
	RM_FileScan fs;
	char *pData;
	PersonRecord *recBuf;

	memset(person_name, 0, sizeof(person_name));
	// val==1 -> show all
	// NO_OP -> No Operation (get all records)
	if (val == 1) {
		if ((rc = fs.OpenScan(fh, STRING, sizeof(char[32]), 
			offsetof(PersonRecord, name), NO_OP, &person_name, NO_HINT)))
			return (rc);
	}
	// val==other -> delete, update, search
	// EQ_OP -> Equal Operation (get the record equal to the parameter)
	else {
		printf("Input a name that you want to search:");
		scanf("%31s", person_name);//personname 

		if ((rc = fs.OpenScan(fh, STRING, sizeof(char[32]), 
			offsetof(PersonRecord, name), EQ_OP, &person_name, NO_HINT)))
			return (rc);
	}

	// for each record in the file
	printf("***************************\n");
	for (rc = fs.GetNextRec(rec), n = 0;
		rc == 0;
		rc = fs.GetNextRec(rec), n++) {

		// val==0 -> search,  val==1 -> show all
		if (val == 0 || val == 1)
			PrintRMRecord(rec);
		// val==2 -> delete
		else if (val == 2) {
			rec.GetRid(rid);
			fh.DeleteRec(rid);
		}
		// val==3 -> update
		else if (val == 3) {
			printf("This is record you want update.\n");
			PrintRMRecord(rec);

			rec.GetData(pData);
			recBuf = (PersonRecord *)pData;

			printf("***You are not allowed to change the name***\n");
			printf("Input a new address : ");
			scanf("%63s", recBuf->addr);
			printf("Input a new telnum : ");
			scanf("%31s", recBuf->telnum);
			printf("Input a new email : ");
			scanf("%31s", recBuf->email);

			fh.UpdateRec(rec);
		}

	}

	if (rc != RM_EOF)
		return (rc);

	if ((rc = fs.CloseScan()))
		return (rc);


	if (val == 2)
		printf("******delete complete.******\n");
	else if (val == 3)
		printf("******update complete.******\n");
	else
		printf("******%d records found******\n", n);

	// Return ok
	return (0);
}

//
// Desc: Print the TestRec record components
//
void PrintRMRecord(RM_Record &rec)
{
	RID rid;
	char *pData;
	PageNum pn;
	SlotNum sn;
	PersonRecord *recBuf;

	rec.GetRid(rid);
	rec.GetData(pData);
	recBuf = (PersonRecord *)pData;

	rid.GetPageNum(pn);
	rid.GetSlotNum(sn);


	printf("(%d,%d) [%s, %s, %s, %s]\n", pn, sn, 
			recBuf->name, recBuf->addr, recBuf->telnum, recBuf->email);
}

