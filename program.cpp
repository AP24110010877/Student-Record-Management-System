#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>   // for getch()
#define STUD_FILE "students.txt"
#define CRE_FILE  "credentials.txt"
char currentUser[50];
char currentRole[20];
void signup();
int  login();
int  isAdmin();
int  isStaff();
int  isGuest();
void adminMenu();
void staffMenu();
void guestMenu();
void addStudent();
void displayStudents();
void searchStudent();
void deleteStudent();
void updateMarks();
void getPassword(char *pass);
int  usernameExists(const char *username);
int  parseStudentLine(const char *line,
                      int *roll, char *name,
                      char *course, char *degree,
                      int *sem, float *mark);
/* READ PASSWORD AS ***** (WINDOWS / conio.h) */
void getPassword(char *pass) {
    int i = 0;
    char ch;

    while (1) {
        ch = getch();        // read char without echo
        if (ch == 13) {      // Enter key
            break;
        } else if (ch == 8) { // Backspace
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else {
            if (i < 49) {    // avoid overflow
                pass[i++] = ch;
                printf("*");
            }
        }
    }
    pass[i] = '\0';
}

/* CHECK IF USERNAME ALREADY EXISTS */
int usernameExists(const char *username) {
    FILE *fp = fopen(CRE_FILE, "r");
    if (!fp) return 0; // file doesn't exist yet

    char u[50], p[50], r[20];
    while (fscanf(fp, "%s %s %s", u, p, r) == 3) {
        if (strcmp(u, username) == 0) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

/* SIGN UP */
void signup() {
    char username[50], password[50], role[20];
    printf("Enter Username: ");
    scanf("%s", username);

    // prevent duplicate usernames
    if (usernameExists(username)) {
        printf("Error: Username already exists. Please choose another.\n");
        return;
    }

    printf("Enter Password: ");
    getPassword(password);   // masked input
    printf("\n");

    printf("Enter Role (admin/staff/guest): ");
    scanf("%s", role);

    FILE *fp = fopen(CRE_FILE, "a");
    if (!fp) {
        printf("Error opening credentials file!\n");
        return;
    }

    // store username, password, role
    fprintf(fp, "%s %s %s\n", username, password, role);
    fclose(fp);

    printf("Signup successful! Please login.\n");
}

/* LOGIN with 3 attempts */
int login() {
    char u[50], p[50], r[20];
    char inUser[50], inPass[50];
    int attempts = 0;

    while (attempts < 3) {
        printf("\n====== LOGIN ======\n");
        printf("USERNAME: ");
        scanf("%s", inUser);

        printf("PASSWORD: ");
        getPassword(inPass);     // masked input
        printf("\n");

        FILE *fp = fopen(CRE_FILE, "r");
        if (!fp) {
            printf("No credentials found. Sign up first.\n");
            return 0;
        }

        while (fscanf(fp, "%s %s %s", u, p, r) == 3) {
            if (strcmp(inUser, u) == 0 && strcmp(inPass, p) == 0) {
                strcpy(currentUser, u);
                strcpy(currentRole, r);
                fclose(fp);
                printf("Logged in as '%s' with role '%s'\n",
                       currentUser, currentRole);
                return 1;  // SUCCESS
            }
        }

        fclose(fp);

        attempts++;
        if (attempts < 3) {
            printf("Invalid username or password! Attempts left: %d\n",
                   3 - attempts);
        }
    }

    printf("Too many failed attempts. Access Denied!\n");
    return 0;  // FAILED AFTER 3 ATTEMPTS
}

/* ROLE CHECKERS */
int isAdmin() {
    return (strcmp(currentRole, "admin") == 0 ||
            strcmp(currentRole, "ADMIN") == 0);
}

int isStaff() {
    return (strcmp(currentRole, "staff") == 0 ||
            strcmp(currentRole, "STAFF") == 0);
}

int isGuest() {
    return (strcmp(currentRole, "guest") == 0 ||
            strcmp(currentRole, "GUEST") == 0);
}

/* Parse a student line (ignores extra stuff at the end, like old datetime) */
int parseStudentLine(const char *line,
                     int *roll, char *name,
                     char *course, char *degree,
                     int *sem, float *mark) {
    // name max 49 chars, course 19, degree 19
    return (sscanf(line, "%d %49s %19s %19s %d %f",
                   roll, name, course, degree, sem, mark) == 6);
}

/* ADD STUDENT (name WITHOUT spaces to keep file format simple) */
void addStudent() {
    FILE *fp = fopen(STUD_FILE, "a");
    if (!fp) {
        printf("Cannot open student file!\n");
        return;
    }

    int roll, sem;
    float mark;
    char name[50], course[20], degree[20];

    printf("Enter Roll Number: ");
    scanf("%d", &roll);

    printf("Enter Name (no spaces): ");
    scanf("%s", name);

    printf("Enter Course: ");
    scanf("%s", course);

    printf("Enter Degree: ");
    scanf("%s", degree);

    printf("Enter Semester: ");
    scanf("%d", &sem);

    printf("Enter Marks: ");
    scanf("%f", &mark);

    // Write: roll name course degree sem marks
    fprintf(fp, "%d %s %s %s %d %.2f\n",
            roll, name, course, degree, sem, mark);

    fclose(fp);

    printf("Student Record Added Successfully!\n");
}

/* DISPLAY STUDENTS (clean table, robust reading) */
void displayStudents() {
    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("No Student Records Found!\n");
        return;
    }

    int roll, sem;
    float mark;
    char name[50], course[20], degree[20];
    char line[256];

    printf("\n%-6s %-12s %-10s %-10s %-5s %-8s\n",
           "Roll", "Name", "Course", "Degree", "Sem", "Marks");
    printf("----------------------------------------------------------\n");

    while (fgets(line, sizeof(line), fp)) {
        if (parseStudentLine(line, &roll, name, course, degree, &sem, &mark)) {
            printf("%-6d %-12s %-10s %-10s %-5d %-8.2f\n",
                   roll, name, course, degree, sem, mark);
        }
        // malformed lines are silently ignored
    }

    fclose(fp);
}

/* SEARCH STUDENT */
void searchStudent() {
    int choice;
    printf("Search by (1.Roll / 2.Name): ");
    scanf("%d", &choice);

    int roll, sem, found = 0;
    float mark;
    char name[50], course[20], degree[20], searchName[50];
    char line[256];

    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("No records.\n");
        return;
    }

    if (choice == 1) {
        int sroll;
        printf("Enter Roll: ");
        scanf("%d", &sroll);

        while (fgets(line, sizeof(line), fp)) {
            if (parseStudentLine(line, &roll, name, course, degree, &sem, &mark)) {
                if (roll == sroll) {
                    printf("Found: %d %s %s %s %d %.2f\n",
                           roll, name, course, degree, sem, mark);
                    found = 1;
                    break;
                }
            }
        }
    } else {
        printf("Enter Name (no spaces): ");
        scanf("%s", searchName);

        while (fgets(line, sizeof(line), fp)) {
            if (parseStudentLine(line, &roll, name, course, degree, &sem, &mark)) {
                if (strcmp(name, searchName) == 0) {
                    printf("Found: %d %s %s %s %d %.2f\n",
                           roll, name, course, degree, sem, mark);
                    found = 1;
                }
            }
        }
    }

    if (!found)
        printf("Student Not Found!\n");

    fclose(fp);
}

/* DELETE STUDENT */
void deleteStudent() {
    int droll;
    printf("Enter Roll Number to Delete: ");
    scanf("%d", &droll);

    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("No Records Found!\n");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        printf("Error opening temp file!\n");
        fclose(fp);
        return;
    }

    int roll, sem, deleted = 0;
    float mark;
    char name[50], course[20], degree[20];
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if (parseStudentLine(line, &roll, name, course, degree, &sem, &mark)) {
            if (roll == droll) {
                deleted = 1;
                // skip writing this record
                continue;
            }
            // write normalized line (without any old extra fields)
            fprintf(temp, "%d %s %s %s %d %.2f\n",
                    roll, name, course, degree, sem, mark);
        } else {
            // line not parsed as student record – keep it as is
            fputs(line, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    remove(STUD_FILE);
    rename("temp.txt", STUD_FILE);

    if (deleted)
        printf("Student Record Deleted Successfully!\n");
    else
        printf("Roll Number Not Found!\n");
}

/* UPDATE MARKS */
void updateMarks() {
    int uroll;
    printf("Enter Roll Number to Update Marks: ");
    scanf("%d", &uroll);

    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("No Records Found!\n");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        printf("Error opening temp file!\n");
        fclose(fp);
        return;
    }

    int roll, sem, updated = 0;
    float mark, newMarks;
    char name[50], course[20], degree[20];
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if (parseStudentLine(line, &roll, name, course, degree, &sem, &mark)) {
            if (roll == uroll) {
                printf("Current Record: %d %s %s %s %d %.2f\n",
                       roll, name, course, degree, sem, mark);

                printf("Enter New Marks: ");
                scanf("%f", &newMarks);

                fprintf(temp, "%d %s %s %s %d %.2f\n",
                        roll, name, course, degree, sem, newMarks);
                updated = 1;
            } else {
                fprintf(temp, "%d %s %s %s %d %.2f\n",
                        roll, name, course, degree, sem, mark);
            }
        } else {
            // malformed line, keep as-is
            fputs(line, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    remove(STUD_FILE);
    rename("temp.txt", STUD_FILE);

    if (updated)
        printf("Marks Updated Successfully!\n");
    else
        printf("Roll Number Not Found!\n");
}

/* ADMIN MENU */
void adminMenu() {
    int menu;
    while (1) {
        printf("\n===== ADMIN PORTAL =====\n");
        printf("1. Add Student\n");
        printf("2. Display Students\n");
        printf("3. Search Student\n");
        printf("4. Delete Student\n");
        printf("5. Update Marks\n");
        printf("6. Logout\n");
        printf("Choose: ");
        scanf("%d", &menu);

        if (menu == 1) {
            addStudent();
        } else if (menu == 2) {
            displayStudents();
        } else if (menu == 3) {
            searchStudent();
        } else if (menu == 4) {
            deleteStudent();
        } else if (menu == 5) {
            updateMarks();
        } else if (menu == 6) {
            printf("Logging out...\n");
            break;
        } else {
            printf("Invalid choice!\n");
        }
    }
}

/* STAFF MENU */
void staffMenu() {
    int menu;
    while (1) {
        printf("\n===== STAFF PORTAL =====\n");
        printf("1. Display Students\n");
        printf("2. Search Student\n");
        printf("3. Update Marks\n");
        printf("4. Logout\n");
        printf("Choose: ");
        scanf("%d", &menu);

        if (menu == 1) {
            displayStudents();
        } else if (menu == 2) {
            searchStudent();
        } else if (menu == 3) {
            updateMarks();
        } else if (menu == 4) {
            printf("Logging out...\n");
            break;
        } else {
            printf("Invalid choice!\n");
        }
    }
}

/* GUEST MENU */
void guestMenu() {
    int menu;
    while (1) {
        printf("\n===== GUEST PORTAL =====\n");
        printf("1. Display Students\n");
        printf("2. Search Student\n");
        printf("3. Logout\n");
        printf("Choose: ");
        scanf("%d", &menu);

        if (menu == 1) {
            displayStudents();
        } else if (menu == 2) {
            searchStudent();
        } else if (menu == 3) {
            printf("Logging out...\n");
            break;
        } else {
            printf("Invalid choice!\n");
        }
    }
}

/* MAIN */
int main() {
    int choice;
    printf("1. Sign Up\n2. Login\nChoose Option: ");
    scanf("%d", &choice);

    if (choice == 1) {
        signup();
    }

    if (!login()) {
        printf("Invalid login!\n");
        return 0;
    }

    printf("\nLogin Successful! User: %s | Role: %s\n",
           currentUser, currentRole);

    if (isAdmin()) {
        adminMenu();
    } else if (isStaff()) {
        staffMenu();
    } else if (isGuest()) {
        guestMenu();
    } else {
        printf("Unknown role '%s', going to guest portal by default.\n",
               currentRole);
        guestMenu();
    }

    printf("Program Ended.\n");
    return 0;
}

