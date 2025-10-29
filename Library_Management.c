#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// =================== STRUCT DEFINITIONS ===================

// --- Author Struct ---
typedef struct Author {
    int id;
    char firstName[50];
    char lastName[50];
    struct Author* next;
} Author;

typedef struct BookAuthor {
    char isbn[14];
    int authorID;
} BookAuthor;

typedef struct {
    BookAuthor* list;
    int count;
} BookAuthorManager;

// --- Student Struct ---
typedef struct Student {
    char id[9]; // 8 digits + null char
    char firstName[50];
    char lastName[50];
    int points;
    struct Student* next;
    struct Student* prev;
} Student;

// --- BookCopy Struct ---
typedef struct BookCopy {
    char label[30];       // ISBN_1, ISBN_2, etc.
    char status[20];      // "RAFTA" or student ID
    struct BookCopy* next;
} BookCopy;

// --- Book Struct ---
typedef struct Book {
    char title[100];
    char isbn[14];        // 13 digits + null terminator
    int quantity;
    BookCopy* copies;
    struct Book* next;
} Book;

// --- Loan Record Struct ---
typedef struct LoanRecord {
    char studentID[9];
    char label[30];   // KitapEtiketNO
    int type;         // 0 = loan, 1 = return
    char date[11];    // DD-MM-YYYY
    struct LoanRecord* next;
} LoanRecord;

// =================== FUNCTION POINTER STRUCTS ===================

typedef void (*StudentOpFunc)(Student**, LoanRecord**, Book*);
typedef struct {
    int option;
    const char* label;
    StudentOpFunc func;
} StudentOperation;

typedef void (*AuthorOpFunc)(Author**, BookAuthorManager*, Book*);

typedef struct {
    int option;
    const char* label;
    AuthorOpFunc func;
} AuthorOperation;

typedef void (*BookOpFunc)(Book**, LoanRecord**, Author*, BookAuthorManager*);
typedef struct {
    int option;
    const char* label;
    BookOpFunc func;
} BookOperation;

void writeStudentsToFile(Student* head);
void writeBooksToFile(Book* head);
void writeLoansToFile(LoanRecord* head);

Student* addStudent(Student* head, char* id, char* first, char* last);

int studentExists(Student* head, const char* id) {
    while (head) {
        if (strcmp(head->id, id) == 0) {
            return 1;  // ID already exists
        }
        head = head->next;
    }
    return 0;
}
LoanRecord* addLoanRecord(LoanRecord* head, const char* studentID, const char* label, int type, const char* date);

int borrowBook(Student* studentList, Book* bookList, LoanRecord** loanList, const char* studentID, const char* isbn, const char* date) {
    // 1. Check student exists
    Student* s = studentList;
    while (s && strcmp(s->id, studentID) != 0) s = s->next;
    if (!s) {
        printf("Student not found.\n");
        return 0;
    }

    if (s->points <= 0) {
        printf("Student has insufficient points.\n");
        return 0;
    }

    // 2. Find a free copy of the book
    Book* b = bookList;
    while (b && strcmp(b->isbn, isbn) != 0) b = b->next;
    if (!b) {
        printf("Book not found.\n");
        return 0;
    }

    BookCopy* copy = b->copies;
    while (copy && strcmp(copy->status, "RAFTA") != 0) copy = copy->next;

    if (!copy) {
        printf("OPERATION FAILED: All copies are currently borrowed.\n");
        return 0;
    }

    // 3. Mark as borrowed
    strcpy(copy->status, studentID);

    // 4. Record transaction
  *loanList = addLoanRecord(*loanList, studentID, copy->label, 0, date);


    writeLoansToFile(*loanList);
    writeBooksToFile(bookList);

    printf("Book %s successfully borrowed by %s.\n", copy->label, studentID);
    return 1;
}

int returnBook(Student* studentList, Book* bookList, LoanRecord** loanList, const char* studentID, const char* label, const char* returnDate) {
    // 1. Check student exists
    Student* s = studentList;
    while (s && strcmp(s->id, studentID) != 0) s = s->next;
    if (!s) {
        printf("Student not found.\n");
        return 0;
    }

    // 2. Find the book copy
    Book* b = bookList;
    BookCopy* c = NULL;
    while (b) {
        c = b->copies;
        while (c) {
            if (strcmp(c->label, label) == 0) break;
            c = c->next;
        }
        if (c) break;
        b = b->next;
    }

    if (!c) {
        printf("Book copy not found.\n");
        return 0;
    }

    if (strcmp(c->status, studentID) != 0) {
        printf("This book is not borrowed by this student.\n");
        return 0;
    }

    // 3. Find matching loan date
    LoanRecord* l = *loanList;
    LoanRecord* match = NULL;
    while (l) {
        if (l->type == 0 && strcmp(l->label, label) == 0 && strcmp(l->studentID, studentID) == 0) {
            match = l;
        }
        l = l->next;
    }

    // 4. Calculate delay
    if (match) {
        int days = daysBetween(match->date, returnDate);
        if (days > 15) {
            printf("Returned late. -10 penalty applied.\n");
            s->points -= 10;
            if (s->points < 0) s->points = 0;
            writeStudentsToFile(studentList);
        }
    }

    // 5. Mark book as on shelf
    strcpy(c->status, "RAFTA");

    // 6. Record return
   *loanList = addLoanRecord(*loanList, studentID, label, 1, returnDate);


    writeLoansToFile(*loanList);
    writeBooksToFile(bookList);

    printf("Book %s successfully returned.\n", label);
    return 1;
}
 

 
 // Author function prototypes
Author* addAuthor(Author* head, char* first, char* last);
void writeAuthorsToFile(Author* head);
Author* deleteAuthor(Author* head, int id, BookAuthorManager* manager);
Author* readAuthorsFromFile(void);
void viewAuthorInfo(const char* firstName, Author* authorList, Book* bookList, BookAuthorManager* manager);
Author* updateAuthor(Author* head, int id);

// student functions  prototypes 
void showStudentInfo(Student* head, LoanRecord* loans,  const char* id);
void listStudentsWithUnreturnedBooks(Student* students, LoanRecord* loans);
void listPenalizedStudents(Student* students, LoanRecord* loans);
void listAllStudents(Student* students);


// book function prototypes

Book* addBook(Book* head, char* title, char* isbn, int quantity);
Book* deleteBookByISBN(Book* head, const char* isbn);
int updateBookTitle(Book* head, const char* isbn, const char* newTitle);
void showBookInfoByTitle(Book* head, const char* title);
void listBooksOnShelf(Book* head);
void listOverdueBooks(LoanRecord* loans);


typedef void (*AuthorOpFunc)(Author**, BookAuthorManager*, Book*) ; 

  void op_addAuthor(Author** list, BookAuthorManager* manager, Book* bookList) {
    char first[50], last[50];
    printf("Enter first name: ");
    scanf("%s", first);
    printf("Enter last name: ");
    scanf("%s", last);
    *list = addAuthor(*list, first, last);
    writeAuthorsToFile(*list);
    printf("Author added.\n");
}


void op_deleteAuthor(Author** list, BookAuthorManager* manager,Book* bookList) {
    int id;
    printf("Enter author ID to delete: ");
    scanf("%d", &id);
    *list = deleteAuthor(*list, id, manager);
}

void op_updateAuthor(Author** list, BookAuthorManager* manager,Book* bookList) {
    int id;
    printf("Enter author ID to update: ");
    scanf("%d", &id);
    updateAuthor(*list, id);
    writeAuthorsToFile(*list);
}

void op_viewAuthor(Author** list, BookAuthorManager* manager, Book* bookList) {
    char first[50];
    printf("Enter author's first name: ");
    scanf("%s", first);
    viewAuthorInfo(first, *list, bookList, manager);
}

void op_listAllAuthors(Author** list, BookAuthorManager* manager,Book* bookList) {
    printf("\n--- ALL AUTHORS ---\n");
    Author* a;
    for (a = *list; a != NULL; a = a->next) {
        printf("ID: %d | Name: %s %s\n", a->id, a->firstName, a->lastName);
    }
}


// =================== Author Functions ===================
Author* addAuthor(Author* head, char* first, char* last) {
    Author* newAuthor = malloc(sizeof(Author));

    // Find the current maximum ID in the list
    int maxID = 0;
    Author* temp = head;
    while (temp != NULL) {
        if (temp->id > maxID) maxID = temp->id;
        temp = temp->next;
    }

    newAuthor->id = maxID + 1; // Assign next ID
    strcpy(newAuthor->firstName, first);
    strcpy(newAuthor->lastName, last);
    newAuthor->next = NULL;

    // Insert into sorted list (keep your existing code here)
    if (!head) return newAuthor;

    Author* current = head;
    Author* prev = NULL;
    while (current && current->id < newAuthor->id) {
        prev = current;
        current = current->next;
    }
    if (!prev) {
        newAuthor->next = head;
        return newAuthor;
    }
    prev->next = newAuthor;
    newAuthor->next = current;
    return head;
}

void addBookAuthorMapping(BookAuthorManager* manager, const char* isbn, int authorID) {
    manager->list = realloc(manager->list, (manager->count + 1) * sizeof(BookAuthor));
    strcpy(manager->list[manager->count].isbn, isbn);
    manager->list[manager->count].authorID = authorID;
    manager->count++;
}

void writeBookAuthorCSV(BookAuthorManager* manager) {
    FILE* f = fopen("KitapYazar.csv", "w");
    if (!f) {
        printf("Couldn't write to KitapYazar.csv\n");
        return;
    }
    int i;
    for (i = 0; i < manager->count; i++) {
        fprintf(f, "%s,%d\n", manager->list[i].isbn, manager->list[i].authorID);
}
    fclose(f);
}

void writeAuthorsToFile(Author* head) {
    FILE* file = fopen("Yazarlar.csv", "w");
    while (head) {
        fprintf(file, "%d,%s,%s\n", head->id, head->firstName, head->lastName);
        head = head->next;
    }
    fclose(file);
}

void readBookAuthorCSV(BookAuthorManager* manager) {
    FILE* f = fopen("KitapYazar.csv", "r");
    if (!f) return;
    char line[100];
    while (fgets(line, sizeof(line), f)) {
        char isbn[14];
        int id;
        sscanf(line, "%13[^,],%d", isbn, &id);
       addBookAuthorMapping(manager, isbn, id);
    }
    fclose(f);
}

void updateBookAuthors(BookAuthorManager* manager, const char* isbn) {
    int i, count = 0;
    for (i = 0; i < manager->count; i++) {
        if (strcmp(manager->list[i].isbn, isbn) == 0) {
            manager->list[i].authorID = -1;
            count++;
        }
    }

    if (count == 0) {
        printf("No existing authors found for ISBN %s.\n", isbn);
    } else {
        printf("%d old author(s) removed for %s.\n", count, isbn);
    }

    int newCount;
    printf("How many new authors for this book? ");
    scanf("%d", &newCount);
int j;
    for (j = 0; j < newCount; j++) {
        int newID;
        printf("Enter author ID #%d: ", j + 1);
        scanf("%d", &newID);
        addBookAuthorMapping(manager, isbn, newID);
    }

    writeBookAuthorCSV(manager);
    printf("Authors for book %s updated.\n", isbn);
    return;
}
 
  
void op_addStudent(Student** list, LoanRecord** loanList, Book* bookList) {
    char id[9], first[50], last[50];
    printf("ID: "); scanf("%s", id);
    if (studentExists(*list, id)) { printf("Exists.\n"); return; }
    printf("First name: "); scanf("%s", first);
    printf("Last name: "); scanf("%s", last);
    *list = addStudent(*list, id, first, last);
    writeStudentsToFile(*list);
}


void op_deleteStudent(Student** list, LoanRecord** loanList, Book* bookList) {
    char id[9];
    printf("Enter ID to delete: ");
    scanf("%s", id);
    if (deleteStudent(list, id)) {
        writeStudentsToFile(*list);
        printf("Deleted.\n");
    } else {
        printf("Not found.\n");
    }
}

void op_updateStudent(Student** list, LoanRecord** loanList, Book* bookList) {
    char id[9], first[50], last[50];
    printf("Enter ID to update: ");
    scanf("%s", id);
    printf("New First Name: ");
    scanf("%s", first);
    printf("New Last Name: ");
    scanf("%s", last);
    if (updateStudent(*list, id, first, last)) {
        writeStudentsToFile(*list);
        printf("Updated.\n");
    } else {
        printf("Not found.\n");
    }
}

void op_viewStudent(Student** list, LoanRecord** loanList, Book* bookList) {
    char id[9];
    printf("Enter student ID: ");
    scanf("%s", id);
    showStudentInfo(*list, *loanList, id);
}



void op_listUnreturned(Student** list, LoanRecord** loanList, Book* bookList) {
    listStudentsWithUnreturnedBooks(*list, *loanList);
}


void op_listPenalized(Student** list, LoanRecord** loanList, Book* bookList) {
    listPenalizedStudents(*list, *loanList);
}

void op_listAllStudents(Student** list, LoanRecord** loanList, Book* bookList) {
    listAllStudents(*list);
}

void op_borrowBook(Student** studentList, LoanRecord** loanList, Book* bookList) {
    char studentID[9], isbn[14], date[11];
    printf("Enter Student ID: ");
    scanf("%s", studentID);
    printf("Enter Book ISBN: ");
    scanf("%s", isbn);
    printf("Enter Date (DD-MM-YYYY): ");
    scanf("%s", date);

    if (borrowBook(*studentList, bookList, loanList, studentID, isbn, date)) {
        printf("Book borrowed successfully.\n");
    }
}

void op_returnBook(Student** studentList, LoanRecord** loanList, Book* bookList) {
    char studentID[9], label[30], date[11];
    printf("Enter Student ID: ");
    scanf("%s", studentID);
    printf("Enter Book Copy Label (e.g., ISBN_1): ");
    scanf("%s", label);
    printf("Enter Return Date (DD-MM-YYYY): ");
    scanf("%s", date);

    if (returnBook(*studentList, bookList, loanList, studentID, label, date)) {
        printf("Book returned successfully.\n");
    }
}


void op_addBook(Book**, LoanRecord**, Author*, BookAuthorManager*);
void op_deleteBook(Book**, LoanRecord**, Author*, BookAuthorManager*);
void op_updateBook(Book**, LoanRecord**, Author*, BookAuthorManager*);
void op_viewBookByTitle(Book**, LoanRecord**, Author*, BookAuthorManager*);
void op_listBooksOnShelf(Book**, LoanRecord**, Author*, BookAuthorManager*);
void op_listAllBooks(Book**, LoanRecord**, Author*, BookAuthorManager*);
void op_listOverdueBooks(Book**, LoanRecord**, Author*, BookAuthorManager*);
void op_addBookAuthorMapping(Book**, LoanRecord**, Author*, BookAuthorManager*);
void op_updateBookAuthors(Book**, LoanRecord**, Author*, BookAuthorManager*);


//table for student

StudentOperation studentOps[] = {
    {1, "Add Student", op_addStudent},
    {2, "Delete Student", op_deleteStudent},
    {3, "Update Student", op_updateStudent},
    {4, "View Student Info", op_viewStudent},
    {5, "List Students with Unreturned Books", op_listUnreturned},
    {6, "List Penalized Students", op_listPenalized},
    {7, "List All Students", op_listAllStudents},
    {8, "Borrow Book", op_borrowBook},
    {9, "Return Book", op_returnBook},
};

// table for authors 

AuthorOperation authorOps[] = {
    {1, "Add Author", op_addAuthor},
    {2, "Delete Author", op_deleteAuthor},
    {3, "Update Author", op_updateAuthor},
    {4, "View Author Info", op_viewAuthor},
    {5, "List All Authors", op_listAllAuthors}
};

//table for book operations
BookOperation bookOps[] = {
    {1, "Add Book", op_addBook},
    {2, "Delete Book", op_deleteBook},
    {3, "Update Book Title", op_updateBook},
    {4, "View Book by Title", op_viewBookByTitle},
    {5, "List Books on Shelf", op_listBooksOnShelf},
    {6, "List Overdue Books", op_listOverdueBooks},
    {7, "List All Books", op_listAllBooks},
    {8, "Add Book-Author Mapping", op_addBookAuthorMapping},
    {9, "Update Book Authors", op_updateBookAuthors},
};


// function to show book authors
void showAuthorsForBook(const char* isbn, Author* authorList, BookAuthorManager* manager) {
    printf("  Authors: ");
    int found = 0;
    int i;
	for (i = 0; i < manager->count; i++) {
        if (strcmp(manager->list[i].isbn, isbn) == 0 && manager->list[i].authorID != -1) {
            Author* a = authorList;
            while (a) {
                if (a->id == manager->list[i].authorID) {
                    printf("%s %s  ", a->firstName, a->lastName);
                    found = 1;
                    break;
                }
                a = a->next;
            }
        }
    }
    if (!found) printf("None");
    printf("\n");
}
   
void markAuthorDeletedInMappings(BookAuthorManager* manager, int deletedAuthorID) {
    int changed = 0;
    int i;
	for (i = 0; i < manager->count; i++) {
        if (manager->list[i].authorID == deletedAuthorID) {
            manager->list[i].authorID = -1;
            changed++;
        }
    }
    if (changed > 0) {
        writeBookAuthorCSV(manager);
        printf("%d mappings updated to -1 for deleted author ID %d.\n", changed, deletedAuthorID);
    } else {
        printf("No mappings found for author ID %d.\n", deletedAuthorID);
    }
}
   

Author* deleteAuthor(Author* head, int id, BookAuthorManager* manager) {
    Author* current = head;
    Author* prev = NULL;

    while (current) {
        if (current->id == id) {
            if (prev) prev->next = current->next;
            else head = current->next;

            free(current);
            markAuthorDeletedInMappings(manager, id);  // <-- Fixed
            writeAuthorsToFile(head);
            printf("Author ID %d deleted.\n", id);
            return head;
        }
        prev = current;
        current = current->next;
    }

    printf("Author ID %d not found.\n", id);
    return head;
}


Author* readAuthorsFromFile() {
    FILE* file = fopen("Yazarlar.csv", "r");
    if (!file) return NULL;

    char line[200];
    Author* head = NULL;
    Author* tail = NULL;

    while (fgets(line, sizeof(line), file)) {
        Author* a = malloc(sizeof(Author));
        sscanf(line, "%d,%49[^,],%49[^\n]", &a->id, a->firstName, a->lastName);
        a->next = NULL;

        if (!head) head = tail = a;
        else {
            tail->next = a;
            tail = a;
        }
    }

    fclose(file);
    return head;
}
// View Author Information
void viewAuthorInfo(const char* firstName, Author* authorList, Book* bookList, BookAuthorManager* manager) {
    Author* author = authorList;
    int found = 0;
    while (author) {
        if (strcmp(author->firstName, firstName) == 0) {
            printf("Author ID: %d\nName: %s %s\n", author->id, author->firstName, author->lastName);
            printf("Books by this author:\n");
            int i;
            for (i = 0; i < manager->count; i++) {
                if (manager->list[i].authorID == author->id && manager->list[i].authorID != -1) {
                    Book* book = bookList;
                    while (book) {
                        if (strcmp(book->isbn, manager->list[i].isbn) == 0) {
                            printf("- %s (ISBN: %s)\n", book->title, book->isbn);
                            break;
                        }
                        book = book->next;
                    }
                }
            }
            found = 1;
            break;
        }
        author = author->next;
    }
    if (!found) {
        printf("Author not found.\n");
    }
}


// Update Author (basic implementation)
Author* updateAuthor(Author* head, int id) {
    Author* current = head;
    while (current) {
        if (current->id == id) {
            char first[50], last[50];
            printf("Enter new first name: ");
            scanf("%s", first);
            printf("Enter new last name: ");
            scanf("%s", last);
            strcpy(current->firstName, first);
            strcpy(current->lastName, last);
            printf("Author updated.\n");
            return head;
        }
        current = current->next;
    }
    printf("Author ID %d not found.\n", id);
    return head;
}


// =================== Student Functions ===================
Student* addStudent(Student* head, char* id, char* first, char* last) {
    Student* newStudent = malloc(sizeof(Student));
    strcpy(newStudent->id, id);
    strcpy(newStudent->firstName, first);
    strcpy(newStudent->lastName, last);
    newStudent->points = 100;
    newStudent->next = NULL;
    newStudent->prev = NULL;

    if (!head) return newStudent;

    Student* tail = head;
    while (tail->next) tail = tail->next;

    tail->next = newStudent;
    newStudent->prev = tail;
    return head;
}

void writeStudentsToFile(Student* head) {
    FILE* file = fopen("Ogrenciler.csv", "w");
    if (!file) {
        printf("Could not open file for writing.\n");
        return;
    }

    while (head) {
        fprintf(file, "%s,%s,%s,%d\n", head->id, head->firstName, head->lastName, head->points);
        head = head->next;
    }

    fclose(file);
}


Student* readStudentsFromFile() {
    FILE* file = fopen("Ogrenciler.csv", "r");
    if (!file) return NULL;

    char line[200];
    Student* head = NULL;
    Student* tail = NULL;

    while (fgets(line, sizeof(line), file)) {
        Student* s = malloc(sizeof(Student));
        sscanf(line, "%8[^,],%49[^,],%49[^,],%d", s->id, s->firstName, s->lastName, &s->points);
        s->next = NULL;
        s->prev = NULL;

        if (!head) head = tail = s;
        else {
            tail->next = s;
            s->prev = tail;
            tail = s;
        }
    }

    fclose(file);
    return head;
}

int deleteStudent(Student** head, const char* id) {
    Student* current = *head;
    while (current) {
        if (strcmp(current->id, id) == 0) {
            if (current->prev) current->prev->next = current->next;
            else *head = current->next;

            if (current->next) current->next->prev = current->prev;

            free(current);
            return 1;  // success
        }
        current = current->next;
    }
    return 0;  // not found
}

int updateStudent(Student* head, const char* id, const char* newFirst, const char* newLast) {
    while (head) {
        if (strcmp(head->id, id) == 0) {
            strcpy(head->firstName, newFirst);
            strcpy(head->lastName, newLast);
            return 1;
        }
        head = head->next;
    }
    return 0;
}

void showStudentInfo(Student* head, LoanRecord* loans, const char* id) {
    while (head) {
        if (strcmp(head->id, id) == 0) {
            printf("\nStudent Info:\n");
            printf("ID: %s\nName: %s %s\nPoints: %d\n", head->id, head->firstName, head->lastName, head->points);
            printf("Loan History:\n");

            while (loans) {
                if (strcmp(loans->studentID, id) == 0) {
                    printf("- %s [%s] on %s\n", loans->label, loans->type == 0 ? "LOAN" : "RETURN", loans->date);
                }
                loans = loans->next;
            }
            return;
        }
        head = head->next;
    }
    printf("Student not found.\n");
}

void listStudentsWithUnreturnedBooks(Student* students, LoanRecord* loans) {
    printf("\n--- Student that havent return books ---\n");
    Student* s;
    for (s = students; s != NULL; s = s->next) {
        int hasUnreturned = 0;
        LoanRecord* l;
        for (l = loans; l != NULL; l = l->next) {
            if (l->type == 0 && strcmp(l->studentID, s->id) == 0) {
                // search for matching return
                LoanRecord* r;
                int returned = 0;
                for (r = loans; r != NULL; r = r->next) {
                    if (r->type == 1 &&
                        strcmp(r->studentID, s->id) == 0 &&
                        strcmp(r->label, l->label) == 0) {
                        returned = 1;
                        break;
                    }
                }
                if (!returned) {
                    hasUnreturned = 1;
                    break;
                }
            }
        }
        if (hasUnreturned) {
            printf("ID: %s | %s %s\n", s->id, s->firstName, s->lastName);
        }
    }
}


void listPenalizedStudents(Student* students, LoanRecord* loans) {
    printf("\n--- Penalized Students ---\n");

    LoanRecord* l;
    for (l = loans; l != NULL; l = l->next) {
        if (l->type == 1) { // return
            LoanRecord* loan = NULL;
            LoanRecord* k = loans;
            while (k) {
                if (k->type == 0 &&
                    strcmp(k->studentID, l->studentID) == 0 &&
                    strcmp(k->label, l->label) == 0) {
                    loan = k;
                    break;
                }
                k = k->next;
            }

            if (loan) {
                int delay = daysBetween(loan->date, l->date);
                if (delay > 15) {
                    Student* s = students;
                    while (s) {
                        if (strcmp(s->id, l->studentID) == 0) {
                            printf("ID: %s | %s %s | Late Return: %d day\n",
                                   s->id, s->firstName, s->lastName, delay);
                            break;
                        }
                        s = s->next;
                    }
                }
            }
        }
    }
}




void listAllStudents(Student* head) {
    printf("\n--- All Students ---\n");
    while (head) {
        printf("ID: %s | Name: %s %s | Points: %d\n", head->id, head->firstName, head->lastName, head->points);
        head = head->next;
    }
}



// book operation functions 

void op_addBook(Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    char title[100], isbn[14];
    int quantity;
    printf("Enter book title: ");
    scanf(" %[^\n]", title);
    printf("Enter ISBN: ");
    scanf("%s", isbn);
    if (bookExists(*bookList, isbn)) {
        printf("Book already exists.\n");
        return;
    }
    printf("Enter quantity: ");
    scanf("%d", &quantity);
    *bookList = addBook(*bookList, title, isbn, quantity);
    writeBooksToFile(*bookList);
    printf("Book added.\n");
}

void op_deleteBook(Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    char isbn[14];
    printf("Enter ISBN to delete: ");
    scanf("%s", isbn);
    *bookList = deleteBookByISBN(*bookList, isbn);
    writeBooksToFile(*bookList);
    printf("Book deleted.\n");
}
void op_updateBook(Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    char isbn[14], newTitle[100];
    printf("Enter ISBN to update: ");
    scanf("%s", isbn);
    printf("New Title: ");
    scanf(" %[^\n]", newTitle);
    if (updateBookTitle(*bookList, isbn, newTitle)) {
        writeBooksToFile(*bookList);
        printf("Book title updated.\n");
    } else {
        printf("Book not found.\n");
    }
}
void op_viewBookByTitle(Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    char title[100];
    printf("Enter title to search: ");
    scanf(" %[^\n]", title);
    showBookInfoByTitle(*bookList, title);
}
void op_listBooksOnShelf(Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    listBooksOnShelf(*bookList);
}
void op_listAllBooks(Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    Book* b;
    BookCopy* c;
    for (b = *bookList; b != NULL; b = b->next) {
        printf("Book: %s, ISBN: %s, Qty: %d\n", b->title, b->isbn, b->quantity);
        showAuthorsForBook(b->isbn, authorList, manager);
        for (c = b->copies; c != NULL; c = c->next) {
            printf("   Copy: %s, Status: %s\n", c->label, c->status);
        }
    }
}
void op_listOverdueBooks(Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    listOverdueBooks(*loanList);
}
void op_addBookAuthorMapping(Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    char isbn[14];
    int authorID;

    printf("Enter Book ISBN: ");
    scanf("%s", isbn);
    printf("Enter Author ID: ");
    scanf("%d", &authorID);

    addBookAuthorMapping(manager, isbn, authorID);
    writeBookAuthorCSV(manager);
    printf("Mapping added: Book %s  Author %d\n", isbn, authorID);
}
void op_updateBookAuthors(Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    char isbn[14];
    printf("Enter Book ISBN to update authors: ");
    scanf("%s", isbn);
    updateBookAuthors(manager, isbn);
}


// =================== Book Functions ===================
BookCopy* createBookCopies(char* isbn, int quantity) {
    BookCopy* head = NULL;
    BookCopy* tail = NULL;
    int i;
    for (i = 1; i <= quantity; i++) {
        BookCopy* copy = malloc(sizeof(BookCopy));
        sprintf(copy->label, "%s_%d", isbn, i);
        strcpy(copy->status, "RAFTA");
        copy->next = NULL;

        if (!head) head = tail = copy;
        else {
            tail->next = copy;
            tail = copy;
        }
    }
    return head;
}

Book* addBook(Book* head, char* title, char* isbn, int quantity) {
    Book* newBook = malloc(sizeof(Book));
    strcpy(newBook->title, title);
    strcpy(newBook->isbn, isbn);
    newBook->quantity = quantity;
    newBook->copies = createBookCopies(isbn, quantity);
    newBook->next = NULL;

    if (!head) return newBook;

    Book* tail = head;
    while (tail->next) tail = tail->next;
    tail->next = newBook;
    return head;
}

Book* deleteBookByISBN(Book* head, const char* isbn) {
    Book* curr = head;
    Book* prev = NULL;

    while (curr) {
        if (strcmp(curr->isbn, isbn) == 0) {
            if (prev) prev->next = curr->next;
            else head = curr->next;

            // free copies
            BookCopy* c = curr->copies;
            while (c) {
                BookCopy* temp = c;
                c = c->next;
                free(temp);
            }
            free(curr);
            return head;
        }
        prev = curr;
        curr = curr->next;
    }

    return head; // not found
}

int updateBookTitle(Book* head, const char* isbn, const char* newTitle) {
    while (head) {
        if (strcmp(head->isbn, isbn) == 0) {
            strcpy(head->title, newTitle);
            return 1;
        }
        head = head->next;
    }
    return 0;
}
int bookExists(Book* head, const char* isbn) {
    while (head) {
        if (strcmp(head->isbn, isbn) == 0)
            return 1;
        head = head->next;
    }
    return 0;
}


void writeBooksToFile(Book* head) {
    FILE* file = fopen("Kitaplar.csv", "w");
    while (head) {
        fprintf(file, "%s,%s,%d\n", head->title, head->isbn, head->quantity);
        BookCopy* copy = head->copies;
        while (copy) {
            fprintf(file, "%s,%s\n", copy->label, copy->status);
            copy = copy->next;
        }
        head = head->next;
    }
    fclose(file);
}

Book* readBooksFromFile() {
    FILE* file = fopen("Kitaplar.csv", "r");
    if (!file) return NULL;

    char line[256];
    Book* bookList = NULL;
    Book* currentBook = NULL;

    while (fgets(line, sizeof(line), file)) {
        // Check if line contains book info or copy info
        if (strchr(line, ',') && !strchr(line, '_')) {
            // New book entry
            Book* b = malloc(sizeof(Book));
            sscanf(line, " %99[^,],%13[^,],%d", b->title, b->isbn, &b->quantity);
            b->copies = NULL;
            b->next = NULL;

            if (!bookList) bookList = currentBook = b;
            else {
                currentBook->next = b;
                currentBook = b;
            }
        } else if (currentBook != NULL) {
            // BookCopy entry
            BookCopy* c = malloc(sizeof(BookCopy));
            sscanf(line, "%[^,],%s", c->label, c->status);
            c->next = NULL;

            if (!currentBook->copies) currentBook->copies = c;
            else {
                BookCopy* copyTail = currentBook->copies;
                while (copyTail->next) copyTail = copyTail->next;
                copyTail->next = c;
            }
        }
    }

    fclose(file);
    return bookList;
}
void showBookInfoByTitle(Book* head, const char* title) {
    while (head) {
        if (strcmp(head->title, title) == 0) {
            printf("Title: %s, ISBN: %s, Quantity: %d\n", head->title, head->isbn, head->quantity);
            BookCopy* c = head->copies;
            while (c) {
                printf("  Copy: %s | Status: %s\n", c->label, c->status);
                c = c->next;
            }
            return;
        }
        head = head->next;
    }
    printf("Book not found.\n");
}
void listBooksOnShelf(Book* head) {
    printf("\n--- Books on Shelf ---\n");
    while (head) {
        BookCopy* c = head->copies;
        while (c) {
            if (strcmp(c->status, "RAFTA") == 0) {
                printf("Book: %s | Copy: %s\n", head->title, c->label);
            }
            c = c->next;
        }
        head = head->next;
    }
}
LoanRecord* readLoansFromFile() {
    FILE* file = fopen("LoanRecords.csv", "r");
    if (!file) return NULL;

    LoanRecord* head = NULL;
    LoanRecord* tail = NULL;
    char line[200];

    while (fgets(line, sizeof(line), file)) {
        LoanRecord* record = malloc(sizeof(LoanRecord));
        sscanf(line, "%8[^,],%29[^,],%d,%10[^\n]",
               record->studentID, record->label, &record->type, record->date);
        record->next = NULL;

        if (!head) head = tail = record;
        else {
            tail->next = record;
            tail = record;
        }
    }

    fclose(file);
    return head;
}

//time functions 

int daysBetween(const char* d1, const char* d2) {
    struct tm tm1 = {0}, tm2 = {0};
    sscanf(d1, "%d-%d-%d", &tm1.tm_mday, &tm1.tm_mon, &tm1.tm_year);
    sscanf(d2, "%d-%d-%d", &tm2.tm_mday, &tm2.tm_mon, &tm2.tm_year);
    tm1.tm_mon--; tm2.tm_mon--;
    tm1.tm_year -= 1900; tm2.tm_year -= 1900;
    time_t time1 = mktime(&tm1);
    time_t time2 = mktime(&tm2);
    return difftime(time2, time1) / (60 * 60 * 24);
}

void listOverdueBooks(LoanRecord* loans) {
    printf("\n--- Overdue Books ---\n");
    LoanRecord* l = loans;
    while (l) {
        if (l->type == 0) {  // Loan
            int returned = 0;
            LoanRecord* r = loans;
            while (r) {
                if (r->type == 1 &&
                    strcmp(r->studentID, l->studentID) == 0 &&
                    strcmp(r->label, l->label) == 0) {
                    int days = daysBetween(l->date, r->date);
                    if (days > 15) {
                        printf("%s Overdue (%d day) | Student: %s\n", l->label, days, l->studentID);
                    }
                    returned = 1;
                    break;
                }
                r = r->next;
            }
            if (!returned) {
                // still not returned, compare with today
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                char today[11];
                sprintf(today, "%02d-%02d-%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

                int days = daysBetween(l->date, today);
                if (days > 15) {
                    printf("%s Still didnt return (%d day passed) | Student: %s\n", l->label, days, l->studentID);
                }
            }
        }
        l = l->next;
    }
}

// =================== Loan Record Functions ===================
LoanRecord* addLoanRecord(LoanRecord* head, const char* studentID, const char* label, int type, const char* date) {
    LoanRecord* newRec = malloc(sizeof(LoanRecord));
    strcpy(newRec->studentID, studentID);
    strcpy(newRec->label, label);
    newRec->type = type;
    strcpy(newRec->date, date);
    newRec->next = NULL;

    if (!head) return newRec;

    LoanRecord* tail = head;
    while (tail->next) tail = tail->next;
    tail->next = newRec;
    return head;
}


void writeLoansToFile(LoanRecord* head) {
    FILE* file = fopen("LoanRecords.csv", "w");
    while (head) {
        fprintf(file, "%s,%s,%d,%s\n", head->studentID, head->label, head->type, head->date);
        head = head->next;
    }
    fclose(file);
}

// before main functions 

void showStudentMenu(StudentOperation* ops, int opCount, Student** list, LoanRecord** loanList, Book* bookList) {
    int choice;
    while (1) {
        printf("\n--- STUDENT OPERATIONS ---\n");
        int i;
		for (i = 0; i < opCount; ++i)
            printf("%d. %s\n", ops[i].option, ops[i].label);
        printf("0. Back\nChoice: ");
        scanf("%d", &choice);
        getchar();

        if (choice == 0) break;

        int found = 0;
        for ( i = 0; i < opCount; ++i) {
            if (ops[i].option == choice) {
                ops[i].func(list, loanList, bookList);
                found = 1;
                break;
            }
        }
        if (!found) printf("Invalid choice.\n");
    }
}


//author void menu before main

void showAuthorMenu(AuthorOperation* ops, int opCount, Author** list, BookAuthorManager* manager, Book* bookList) {
    int choice;
    while (1) {
        printf("\n--- AUTHOR OPERATIONS ---\n");
        int i;
		for (i = 0; i < opCount; ++i)
            printf("%d. %s\n", ops[i].option, ops[i].label);
        printf("0. Back\nChoice: ");
        scanf("%d", &choice);
        getchar();

        if (choice == 0) break;

        int found = 0;
        for (i = 0; i < opCount; ++i) {
            if (ops[i].option == choice) {
              ops[i].func(list, manager, bookList);

                found = 1;
                break;
            }
        }

        if (!found)
            printf("Invalid choice.\n");
    }
}

//book show menu void 

void showBookMenu(BookOperation* ops, int opCount, Book** bookList, LoanRecord** loanList, Author* authorList, BookAuthorManager* manager) {
    int choice;
    while (1) {
        printf("\n--- BOOK OPERATIONS ---\n");
        int i;
		for ( i = 0; i < opCount; ++i)
            printf("%d. %s\n", ops[i].option, ops[i].label);
        printf("0. Back\nChoice: ");
        scanf("%d", &choice);
        getchar();

        if (choice == 0) break;

        int found = 0;
		for (i = 0; i < opCount; ++i) {
            if (ops[i].option == choice) {
                ops[i].func(bookList, loanList, authorList, manager);
                found = 1;
                break;
            }
        }
        if (!found) printf("Invalid choice.\n");
    }
}


// =================== Main Menu ===================
void showMainMenu() {
    printf("\n===== LIBRARY AUTOMATION MENU =====\n");
    printf("1. Author Operations\n");
    printf("2. Student Operations\n");
    printf("3. Book Operations\n");
    printf("4. Exit\n");
    printf("Choice: ");
}

int main() {
    Author* authorList = readAuthorsFromFile();
    Student* studentList = readStudentsFromFile();
    Book* bookList = readBooksFromFile();
    LoanRecord* loanList = readLoansFromFile();
    BookAuthorManager manager = {NULL, 0};
    readBookAuthorCSV(&manager);

    int choice;
    while (1) {
        showMainMenu();
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                showAuthorMenu(authorOps, sizeof(authorOps)/sizeof(AuthorOperation), &authorList, &manager,bookList);
                break;
            case 2:
              showStudentMenu(studentOps, sizeof(studentOps)/sizeof(StudentOperation), &studentList, &loanList, bookList);
                break;
            case 3:
                showBookMenu(bookOps, sizeof(bookOps)/sizeof(BookOperation), &bookList, &loanList, authorList, &manager);
                break;
            case 4:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}


