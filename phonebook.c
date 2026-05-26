/*
 * Phonebook Management System - Linked List Version
 * Compile: gcc phonebook.c -o phonebook.exe
 * Run: .\phonebook.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h> 
// ==================== 哈希表配置 ====================
#define HASH_SIZE 10007  // 质数，减少哈希冲突

typedef struct {
    char dataFile[100];
    char logFile[100];
    int maxNameLen;
    int maxPhoneLen;
    int maxAddrLen;
    int pageSize;
} Config;

Config config = {
    "phonebook.dat",
    "phonebook.log",
    50, 20, 100, 5
};

typedef struct ContactNode {
    char name[50];
    char phone[20];
    char address[100];
    struct ContactNode *next;
} ContactNode;

ContactNode *head = NULL;
// ==================== 哈希表结构 ====================
typedef struct HashNode {
    ContactNode *contact;      // 指向链表中的联系人
    struct HashNode *next;     // 拉链法解决冲突
} HashNode;

// 姓名哈希表
HashNode *hashTable[HASH_SIZE] = {NULL};

// 电话哈希表（新增）
HashNode *phoneHashTable[HASH_SIZE] = {NULL};


// 哈希表函数声明
unsigned int hashString(const char *str);
void hashInsert(ContactNode *contact);
ContactNode* hashSearch(const char *name);
void hashDelete(const char *name);
void freeHashTable();
void freePhoneHashTable(); 
void phoneHashInsert(ContactNode *contact);
ContactNode* phoneHashSearch(const char *phone);
void phoneHashDelete(const char *phone);
int contactCount = 0;

void loadConfig();
void saveDefaultConfig();
void writeLog(const char *operation, const char *detail);
void writeLogSimple(const char *operation, int count);
char* getCurrentTime();
int isValidPhone(const char *phone);
void trimNewline(char *str);
void pressAnyKey();
void clearScreen();
ContactNode* createNode(const char *name, const char *phone, const char *address);
void insertSorted(const char *name, const char *phone, const char *address);
ContactNode* searchByName(const char *name);
void searchContact();
int deleteContact(const char *name);
void deleteContactUI();
void modifyContact();
void displayAll();
void addContact();
void loadFromFile();
void saveToFile();
void freeList();
void exportToCSV();
void displaySorted(int sortType);
void batchImport();

void loadConfig() {
    FILE *fp = fopen("phonebook.ini", "r");
    if (!fp) { saveDefaultConfig(); return; }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char key[50], value[200];
        if (sscanf(line, "%[^=]=%s", key, value) == 2) {
            if (strcmp(key, "data_file") == 0) strcpy(config.dataFile, value);
            else if (strcmp(key, "log_file") == 0) strcpy(config.logFile, value);
            else if (strcmp(key, "max_name_len") == 0) config.maxNameLen = atoi(value);
            else if (strcmp(key, "max_phone_len") == 0) config.maxPhoneLen = atoi(value);
            else if (strcmp(key, "max_addr_len") == 0) config.maxAddrLen = atoi(value);
            else if (strcmp(key, "page_size") == 0) config.pageSize = atoi(value);
        }
    }
    fclose(fp);
}

void saveDefaultConfig() {
    FILE *fp = fopen("phonebook.ini", "w");
    if (!fp) return;
    fprintf(fp, "# Phonebook Config\n");
    fprintf(fp, "data_file=phonebook.dat\n");
    fprintf(fp, "log_file=phonebook.log\n");
    fprintf(fp, "max_name_len=50\n");
    fprintf(fp, "max_phone_len=20\n");
    fprintf(fp, "max_addr_len=100\n");
    fprintf(fp, "page_size=5\n");
    fclose(fp);
}

char* getCurrentTime() {
    static char timeStr[30];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    return timeStr;
}

void writeLog(const char *operation, const char *detail) {
    FILE *fp = fopen(config.logFile, "a");
    if (!fp) return;
    fprintf(fp, "[%s] %s: %s\n", getCurrentTime(), operation, detail);
    fclose(fp);
}

void writeLogSimple(const char *operation, int count) {
    char detail[100];
    sprintf(detail, "Count: %d", count);
    writeLog(operation, detail);
}

int isValidPhone(const char *phone) {
    for (int i = 0; phone[i]; i++) {
        char c = phone[i];
        if (!(isdigit(c) || c == '+' || c == '-' || c == '(' || c == ')' || c == ' ')) return 0;
    }
    return 1;
}

void trimNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}

void pressAnyKey() {
    printf("\nPress any key to continue...");
    getchar();
    getchar();
}

void clearScreen() { system("cls"); }

ContactNode* createNode(const char *name, const char *phone, const char *address) {
    ContactNode *newNode = (ContactNode*)malloc(sizeof(ContactNode));
    if (!newNode) { printf("Memory allocation failed!\n"); return NULL; }
    strcpy(newNode->name, name);
    strcpy(newNode->phone, phone);
    strcpy(newNode->address, address);
    newNode->next = NULL;
    return newNode;
}

void insertSorted(const char *name, const char *phone, const char *address) {
    ContactNode *newNode = createNode(name, phone, address);
    if (!newNode) return;
    if (head == NULL || strcmp(name, head->name) < 0) {
        newNode->next = head;
        head = newNode;
        contactCount++;
        hashInsert(newNode); 
        phoneHashInsert(newNode);
        return;
    }
    ContactNode *curr = head;
    while (curr->next && strcmp(name, curr->next->name) > 0) curr = curr->next;
    newNode->next = curr->next;
    curr->next = newNode;
    contactCount++;
    hashInsert(newNode);
    phoneHashInsert(newNode); 
}

ContactNode* searchByName(const char *name) {
    // 先用哈希表 O(1) 查询
    ContactNode *result = hashSearch(name);
    if (result) return result;
    
    // 降级到链表遍历（理论上不会发生）
    ContactNode *curr = head;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

int deleteContact(const char *name) {
    ContactNode *curr = head, *prev = NULL;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            if (prev) prev->next = curr->next;
            else head = curr->next;
            hashDelete(curr->name);
            phoneHashDelete(curr->phone);  
            free(curr);
            contactCount--;
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
}

void fuzzySearch(const char *keyword) {
    ContactNode *curr = head;
    int found = 0, index = 1;
    printf("\n========== Fuzzy Search ==========\n");
    printf("Keyword: %s\n", keyword);
    while (curr) {
        if (strstr(curr->name, keyword) || strstr(curr->phone, keyword) || strstr(curr->address, keyword)) {
            printf("%d. Name: %s | Phone: %s | Address: %s\n", index++, curr->name, curr->phone, curr->address);
            found = 1;
        }
        curr = curr->next;
    }
    if (!found) printf("No match found!\n");
}

void displayAll() {
    if (head == NULL) { printf("\nPhonebook is empty!\n"); writeLog("Display", "Empty"); return; }
    ContactNode *curr = head;
    int total = contactCount, pageSize = config.pageSize, totalPages = (total+pageSize-1)/pageSize, currentPage = 1;
    char input[10];
    int jumpPage;
    do {
        clearScreen();
        printf("========== Page %d/%d | Total: %d ==========\n", currentPage, totalPages, total);
        int start = (currentPage-1)*pageSize, idx = 0, printed = 0;
        curr = head;
        while (curr && idx < start) { curr = curr->next; idx++; }
        while (curr && printed < pageSize) {
            printf("%d. %s\t%s\t%s\n", start+printed+1, curr->name, curr->phone, curr->address);
            curr = curr->next; printed++;
        }
        printf("[N]Next [P]Prev [G]Go to [Q]Quit: ");
        scanf("%s", input);
        switch(toupper(input[0])) {
            case 'N': if(currentPage < totalPages) currentPage++; break;
            case 'P': if(currentPage > 1) currentPage--; break;
            case 'G': printf("Page(1-%d): ", totalPages); scanf("%d", &jumpPage);
                      if(jumpPage>=1 && jumpPage<=totalPages) currentPage=jumpPage; break;
        }
    } while(toupper(input[0]) != 'Q');
    writeLogSimple("Display", total);
}

void addContact() {
    char name[100], phone[100], address[200];
    clearScreen();
    printf("========== Add Contact ==========\n");
    do { printf("Name: "); getchar(); fgets(name,sizeof(name),stdin); trimNewline(name);
    } while(strlen(name)==0);
    do { printf("Phone: "); fgets(phone,sizeof(phone),stdin); trimNewline(phone);
    } while(!isValidPhone(phone));
    printf("Address: "); fgets(address,sizeof(address),stdin); trimNewline(address);
    if(searchByName(name)) { printf("%s already exists!\n", name); pressAnyKey(); return; }
    insertSorted(name, phone, address);
    printf("Added! Total: %d contacts\n", contactCount);
    saveToFile();
    pressAnyKey();
}

void searchContact() {
    char keyword[100];
    int choice;
    clearScreen();
    printf("1. Exact search (by name)\n");
    printf("2. Fuzzy search\n");
    printf("3. Search by phone (O(1) hash)\n");  // 新增
    printf("Choice: ");
    scanf("%d",&choice); getchar();
    
    if(choice==1) {
        printf("Name: "); fgets(keyword,sizeof(keyword),stdin); trimNewline(keyword);
        ContactNode *c = searchByName(keyword);  // O(1) 哈希表
        if(c) printf("Name:%s\nPhone:%s\nAddress:%s\n", c->name, c->phone, c->address);
        else printf("Not found\n");
    } 
    else if(choice==2) {
        printf("Keyword: "); fgets(keyword,sizeof(keyword),stdin); trimNewline(keyword);
        fuzzySearch(keyword);
    }
    else if(choice==3) {
        printf("Phone number: "); fgets(keyword,sizeof(keyword),stdin); trimNewline(keyword);
        ContactNode *c = phoneHashSearch(keyword);  // O(1) 电话哈希表
        if(c) {
            printf("\n========== Found ==========\n");
            printf("  Name: %s\n", c->name);
            printf("  Phone: %s\n", c->phone);
            printf("  Address: %s\n", c->address);
        } else {
            printf("No contact with phone number: %s\n", keyword);
        }
    }
    
    pressAnyKey();
}

void deleteContactUI() {
    char name[100];
    clearScreen();
    if(head==NULL) { printf("Phonebook is empty!\n"); pressAnyKey(); return; }
    printf("Name to delete: "); getchar(); fgets(name,sizeof(name),stdin); trimNewline(name);
    if(deleteContact(name)) { printf("Deleted!\n"); saveToFile(); }
    else printf("Not found\n");
    pressAnyKey();
}

void modifyContact() {
    char name[100];
    char newPhone[100], newAddress[200];
    
    clearScreen();
    printf("========== Modify Contact ==========\n");
    
    if (head == NULL) {
        printf("Phonebook is empty!\n");
        pressAnyKey();
        return;
    }
    
    printf("Enter name to modify: ");
    getchar();
    fgets(name, sizeof(name), stdin);
    trimNewline(name);
    
    ContactNode *contact = searchByName(name);
    if (!contact) {
        printf("Contact '%s' not found!\n", name);
        pressAnyKey();
        return;
    }
    
    printf("\nCurrent info:\n");
    printf("  Name: %s\n", contact->name);
    printf("  Phone: %s\n", contact->phone);
    printf("  Address: %s\n", contact->address);
    
    printf("\n--- Leave blank to keep current value ---\n");
    
    // 修改电话
    char oldPhone[20];
    strcpy(oldPhone, contact->phone);
    
    printf("New phone (%s): ", contact->phone);
    fgets(newPhone, sizeof(newPhone), stdin);
    trimNewline(newPhone);
    if (strlen(newPhone) > 0 && isValidPhone(newPhone)) {
        // 电话哈希表需要更新：先删除旧的，再插入新的
        phoneHashDelete(oldPhone);
        strcpy(contact->phone, newPhone);
        phoneHashInsert(contact);
    }
    
    // 修改地址
    printf("New address (%s): ", contact->address);
    fgets(newAddress, sizeof(newAddress), stdin);
    trimNewline(newAddress);
    if (strlen(newAddress) > 0) {
        strcpy(contact->address, newAddress);
    }
    
    printf("\nContact modified successfully!\n");
    writeLog("Modify contact", name);
    saveToFile();
    pressAnyKey();
}

void exportToCSV() {
    int choice;
    printf("1. All 2. First 10 3. First 20: ");
    scanf("%d",&choice);
    int limit = (choice==1)?contactCount:(choice==2)?((contactCount>10)?10:contactCount):((contactCount>20)?20:contactCount);
    FILE *fp = fopen("phonebook_export.csv","w");
    if(!fp) return;
    fprintf(fp, "\xEF\xBB\xBFName,Phone,Address\n");
    ContactNode *curr=head;
    int exported=0;
    while(curr && exported<limit) {
        fprintf(fp,"\"%s\",\"\t%s\",\"%s\"\n", curr->name, curr->phone, curr->address);
        curr=curr->next; exported++;
    }
    fclose(fp);
    printf("Exported %d contacts\n",exported);
    pressAnyKey();
}

// 比较函数（给 qsort 用）
int compareByName(const void *a, const void *b) {
    ContactNode *na = *(ContactNode**)a;
    ContactNode *nb = *(ContactNode**)b;
    return strcmp(na->name, nb->name);
}

int compareByPhone(const void *a, const void *b) {
    ContactNode *na = *(ContactNode**)a;
    ContactNode *nb = *(ContactNode**)b;
    return strcmp(na->phone, nb->phone);
}

int compareByAddress(const void *a, const void *b) {
    ContactNode *na = *(ContactNode**)a;
    ContactNode *nb = *(ContactNode**)b;
    return strcmp(na->address, nb->address);
}

void displaySorted(int sortType) {
    if (head == NULL) {
        printf("\nPhonebook is empty!\n");
        return;
    }
    
    // 将链表复制到数组
    ContactNode **arr = (ContactNode**)malloc(contactCount * sizeof(ContactNode*));
    ContactNode *curr = head;
    for (int i = 0; i < contactCount; i++) {
        arr[i] = curr;
        curr = curr->next;
    }
    
    // 使用 qsort 快速排序 O(n log n)
    if (sortType == 1) {
        qsort(arr, contactCount, sizeof(ContactNode*), compareByName);
    } else if (sortType == 2) {
        qsort(arr, contactCount, sizeof(ContactNode*), compareByPhone);
    } else {
        qsort(arr, contactCount, sizeof(ContactNode*), compareByAddress);
    }
    
    printf("\n========== Sorted Result (qsort) ==========\n");
    int displayCount = (contactCount < 20) ? contactCount : 20;
    for (int i = 0; i < displayCount; i++) {
        printf("%d. %s\t%s\t%s\n", i+1, arr[i]->name, arr[i]->phone, arr[i]->address);
    }
    if (contactCount > 20) {
        printf("... Total %d, showing first 20\n", contactCount);
    }
    
    free(arr);
}

void batchImport() {
    char filename[100];
    clearScreen();
    printf("========== Batch Import ==========\n");
    printf("CSV file name (default: import.csv): ");
    getchar();
    fgets(filename, sizeof(filename), stdin);
    trimNewline(filename);
    if(strlen(filename)==0) strcpy(filename, "import.csv");
    FILE *fp = fopen(filename, "r");
    if(!fp) { printf("Cannot open file\n"); pressAnyKey(); return; }
    char line[512];
    fgets(line, sizeof(line), fp);
    int success=0, fail=0;
    while(fgets(line, sizeof(line), fp)) {
        char name[100]={0}, phone[100]={0}, addr[200]={0};
        sscanf(line,"%[^,],%[^,],%[^\n]", name, phone, addr);
        if(name[0]=='"') { memmove(name,name+1,strlen(name)); name[strlen(name)-1]='\0'; }
        if(phone[0]=='"') { memmove(phone,phone+1,strlen(phone)); phone[strlen(phone)-1]='\0'; }
        if(strlen(name) && isValidPhone(phone) && !searchByName(name)) {
            insertSorted(name, phone, addr);
            success++;
        } else fail++;
    }
    fclose(fp);
    printf("Success: %d, Failed: %d\n", success, fail);
    if(success>0) saveToFile();
    pressAnyKey();
}

void saveToFile() {
    FILE *fp = fopen(config.dataFile, "wb");
    if(!fp) return;
    ContactNode *curr=head;
    while(curr) { fwrite(curr, sizeof(ContactNode), 1, fp); curr=curr->next; }
    fclose(fp);
}

void loadFromFile() {
    FILE *fp = fopen(config.dataFile, "rb");
    if(!fp) return;
    fseek(fp,0,SEEK_END);
    long size=ftell(fp);
    if(size%sizeof(ContactNode)!=0) { fclose(fp); return; }
    fseek(fp,0,SEEK_SET);
    ContactNode temp;
    while(fread(&temp,sizeof(ContactNode),1,fp)==1) insertSorted(temp.name, temp.phone, temp.address);
    fclose(fp);
}

void freeList() {
    freeHashTable();       // 释放姓名哈希表
    freePhoneHashTable();  // 释放电话哈希表
    
    ContactNode *curr = head;
    while (curr) {
        ContactNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    head = NULL;
    contactCount = 0;
}

// ==================== 哈希表实现 ====================

// BKDR 哈希函数
unsigned int hashString(const char *str) {
    unsigned int seed = 131;
    unsigned int hash = 0;
    while (*str) {
        hash = hash * seed + (unsigned char)(*str++);
    }
    return hash % HASH_SIZE;
}

// 插入哈希表
void hashInsert(ContactNode *contact) {
    unsigned int index = hashString(contact->name);
    HashNode *newNode = (HashNode*)malloc(sizeof(HashNode));
    if (!newNode) return;
    newNode->contact = contact;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

// 从哈希表查找 O(1)
ContactNode* hashSearch(const char *name) {
    unsigned int index = hashString(name);
    HashNode *curr = hashTable[index];
    while (curr) {
        if (strcmp(curr->contact->name, name) == 0) {
            return curr->contact;
        }
        curr = curr->next;
    }
    return NULL;
}

// 从哈希表删除
void hashDelete(const char *name) {
    unsigned int index = hashString(name);
    HashNode *curr = hashTable[index];
    HashNode *prev = NULL;
    while (curr) {
        if (strcmp(curr->contact->name, name) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                hashTable[index] = curr->next;
            }
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

// 释放哈希表
void freeHashTable() {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode *curr = hashTable[i];
        while (curr) {
            HashNode *temp = curr;
            curr = curr->next;
            free(temp);
        }
        hashTable[i] = NULL;
    }
}

// ==================== 电话哈希表实现 ====================

// 插入电话哈希表
void phoneHashInsert(ContactNode *contact) {
    unsigned int index = hashString(contact->phone);
    HashNode *newNode = (HashNode*)malloc(sizeof(HashNode));
    if (!newNode) return;
    newNode->contact = contact;
    newNode->next = phoneHashTable[index];
    phoneHashTable[index] = newNode;
}

// 从电话哈希表查找 O(1)
ContactNode* phoneHashSearch(const char *phone) {
    unsigned int index = hashString(phone);
    HashNode *curr = phoneHashTable[index];
    while (curr) {
        if (strcmp(curr->contact->phone, phone) == 0) {
            return curr->contact;
        }
        curr = curr->next;
    }
    return NULL;
}

// 从电话哈希表删除
void phoneHashDelete(const char *phone) {
    unsigned int index = hashString(phone);
    HashNode *curr = phoneHashTable[index];
    HashNode *prev = NULL;
    while (curr) {
        if (strcmp(curr->contact->phone, phone) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                phoneHashTable[index] = curr->next;
            }
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

// 释放电话哈希表
void freePhoneHashTable() {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode *curr = phoneHashTable[i];
        while (curr) {
            HashNode *temp = curr;
            curr = curr->next;
            free(temp);
        }
        phoneHashTable[i] = NULL;
    }
}

int main() {
    loadConfig();
    loadFromFile();
    
    int choice;
    
    do {
        clearScreen();
        printf("================================\n");
        printf("     Phonebook System - Linked List\n");
        printf("================================\n");
        printf("  1. Add Contact\n");
        printf("  2. Search Contact\n");
        printf("  3. Delete Contact\n");
        printf("  4. Display All\n");
        printf("  5. Sort Display\n");
        printf("  6. Modify Contact\n");
        printf("  7. Export CSV\n");
        printf("  8. Batch Import\n");
        printf("  9. Exit & Save\n");
        printf("================================\n");
        printf("Total contacts: %d\n", contactCount);
        printf("Choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Please enter a number!\n");
            getchar();
            getchar();
            continue;
        }
        
        switch (choice) {
            case 1: addContact(); break;
            case 2: searchContact(); break;
            case 3: deleteContactUI(); break;
            case 4: displayAll(); break;
            case 5: 
                printf("Sort by: 1.Name 2.Phone 3.Address\n");
                printf("Choice: ");
                int sortType;
                scanf("%d", &sortType);
                displaySorted(sortType);
                pressAnyKey();
                break;
            case 6: modifyContact(); break;
            case 7: exportToCSV(); break;
            case 8: batchImport(); break;
            case 9:
                printf("Saving...\n");
                saveToFile();
                printf("Thank you!\n");
                break;
            default:
                printf("Invalid choice!\n");
                pressAnyKey();
        }
    } while (choice != 9);
    
    freeList();
    return 0;
}
