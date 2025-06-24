#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <ctype.h>
#include <setjmp.h> // For setjmp and longjmp

#define MAX_STUDENTS 100
#define NUM_SUBJECTS 5

typedef struct {
    int id;
    int student_number; // Student number
    char name[50]; // Name
    int grades[NUM_SUBJECTS]; // Grades
    char letter_grades[NUM_SUBJECTS]; // Letter grades
    int total_score; // Total score
    double average; // Average
} Student;

Student students[MAX_STUDENTS];
int num_students = 0;

const char *subject_names[NUM_SUBJECTS] = {"Korean", "English", "Math", "Science", "Korean History"};

jmp_buf mainMenuJmpBuf; // For longjmp to main menu

int mainMenu();
void studentRegistration();
void viewStudents();
void modifyStudentInfo();
void deleteStudent();
char assignLetterGrade(int score);
void displayStudents(Student *list, int count, int return_code);
void sortStudentsByName(int order);
void sortStudentsByNumber(int order);
void sortStudentsByTotalScore(int order);
void searchOutput();
void sortedOutput();
void editStudent(Student *s);

int getIntegerInput(const char *prompt);
void getStringInput(const char *prompt, char *buffer, int buffer_size);

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Check if terminal supports colors
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_WHITE); // For highlighted menu items
        init_pair(2, COLOR_WHITE, COLOR_BLUE);  // For headers and titles
    }

    // Main loop
    if (setjmp(mainMenuJmpBuf) != 0) {
        // After longjmp, clear the screen
        clear();
    }

    int exit_program = 0;
    while (!exit_program) {
        exit_program = mainMenu();
    }

    // Clean up ncurses
    endwin();
    return 0;
}

int mainMenu() {
    int highlight = 0;
    int choice = -1;
    int c;
    char *choices[] = {
            "1. Register Student",
            "2. View Students",
            "3. Modify Student Info",
            "4. Delete Student",
            "5. Exit Program"
    };
    int n_choices = sizeof(choices) / sizeof(char *);

    while(1) {
        choice = -1; // Reset choice
        clear();
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        // Draw border
        box(stdscr, 0, 0);

        // Title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(1, (cols - strlen("Student Management System"))/2, "Student Management System");
        attroff(COLOR_PAIR(2) | A_BOLD);

        // Separator
        mvhline(2, 1, ACS_HLINE, cols - 2);

        // Menu
        int start_row = 4;
        for (int i = 0; i < n_choices; ++i) {
            if (i == highlight) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(start_row + i, (cols - strlen(choices[i]))/2, "%s", choices[i]);
                attroff(A_REVERSE | A_BOLD);
            } else {
                mvprintw(start_row + i, (cols - strlen(choices[i]))/2, "%s", choices[i]);
            }
        }

        // Instructions
        attron(A_DIM);
        mvprintw(rows - 2, 2, "Use arrow keys to navigate, Enter to select.");
        attroff(A_DIM);

        c = getch();
        switch(c) {
            case KEY_UP:
                highlight = (highlight - 1 + n_choices) % n_choices;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_choices;
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                break;
        }
        if (choice != -1) { // Enter key pressed
            switch(choice) {
                case 0:
                    studentRegistration();
                    break;
                case 1:
                    viewStudents();
                    break;
                case 2:
                    modifyStudentInfo();
                    break;
                case 3:
                    deleteStudent();
                    break;
                case 4:
                    return 1; // Exit program
                default:
                    break;
            }
        }
    }
}

void studentRegistration() {
    echo();
    curs_set(1); // Show cursor
    Student s;
    s.id = num_students + 1;
    s.total_score = 0;

    // Get terminal size
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Get student information
    clear();
    // Draw border
    box(stdscr, 0, 0);
    // Title
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(1, (cols - strlen("Student Registration"))/2, "Student Registration");
    attroff(COLOR_PAIR(2) | A_BOLD);

    mvhline(2, 1, ACS_HLINE, cols - 2);

    int start_row = 4;
    mvprintw(start_row, 2, "Please enter the following information:");

    // Get student number
    s.student_number = getIntegerInput("Student Number: ");

    // Get name
    getStringInput("Name: ", s.name, sizeof(s.name));

    // Get grades
    for (int i = 0; i < NUM_SUBJECTS; ++i) {
        char prompt[50];
        sprintf(prompt, "%s Grade: ", subject_names[i]);
        s.grades[i] = getIntegerInput(prompt);
        s.total_score += s.grades[i];
        // Assign letter grades
        s.letter_grades[i] = assignLetterGrade(s.grades[i]);
    }
    s.average = s.total_score / (double)NUM_SUBJECTS;

    // Add student to the list
    students[num_students++] = s;

    // Completion notification
    mvprintw(start_row + NUM_SUBJECTS * 2 + 2, 2, "Student registration completed!");
    attron(A_DIM);
    mvprintw(rows - 2, 2, "Press Enter to return to the menu.");
    attroff(A_DIM);
    getch();
    noecho();
    curs_set(0); // Hide cursor
}

char assignLetterGrade(int score) {
    if (score >= 90 && score <= 100)
        return 'A';
    else if (score >= 80 && score < 90)
        return 'B';
    else if (score >= 70 && score < 80)
        return 'C';
    else if (score >= 60 && score < 70)
        return 'D';
    else
        return 'F';
}

void viewStudents() {
    int highlight = 0;
    int choice = -1;
    int c;
    char *choices[] = {
            "1. Display All",
            "2. Display Sorted",
            "3. Search and Display",
            "4. Return to Menu"
    };
    int n_choices = sizeof(choices) / sizeof(char *);
    while(1) {
        choice = -1; // Reset choice
        clear();
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        // Draw border
        box(stdscr, 0, 0);

        // Title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(1, (cols - strlen("View Students"))/2, "View Students");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(2, 1, ACS_HLINE, cols - 2);

        // Menu
        int start_row = 4;
        for (int i = 0; i < n_choices; ++i) {
            if (i == highlight) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(start_row + i, (cols - strlen(choices[i]))/2, "%s", choices[i]);
                attroff(A_REVERSE | A_BOLD);
            } else {
                mvprintw(start_row + i, (cols - strlen(choices[i]))/2, "%s", choices[i]);
            }
        }

        // Instructions
        attron(A_DIM);
        mvprintw(rows - 2, 2, "Use arrow keys to navigate, Enter to select.");
        attroff(A_DIM);

        c = getch();
        switch(c) {
            case KEY_UP:
                highlight = (highlight - 1 + n_choices) % n_choices;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_choices;
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                break;
        }
        if (choice != -1) // Enter key pressed
        {
            switch(choice) {
                case 0:
                    displayStudents(students, num_students, 1); // return_code = 1 (Return to View Students)
                    break;
                case 1:
                    sortedOutput();
                    break;
                case 2:
                    searchOutput();
                    break;
                case 3:
                    return; // Return to main menu
                default:
                    break;
            }
        }
    }
}

void displayStudents(Student *list, int count, int return_code) {
    clear();
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Draw border
    box(stdscr, 0, 0);

    // Title
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(1, (cols - strlen("Student List"))/2, "Student List");
    attroff(COLOR_PAIR(2) | A_BOLD);

    mvhline(2, 1, ACS_HLINE, cols - 2);

    int start_row = 4;

    if (count == 0) {
        mvprintw(start_row, 2, "No registered students.");
    } else {
        // Print column headers
        int col = 2;
        mvprintw(start_row, col, "%-3s ", "ID");
        col += 4;
        mvprintw(start_row, col, "%-9s ", "Number");
        col += 10;
        mvprintw(start_row, col, "%-14s ", "Name");
        col += 15;
        for (int i = 0; i < NUM_SUBJECTS; ++i) {
            mvprintw(start_row, col, "%-15s ", subject_names[i]);
            col += 16;
        }
        mvprintw(start_row, col, "%-6s %-7s", "Total", "Average");
        mvhline(start_row + 1, 1, ACS_HLINE, cols - 2);

        // Print students
        for (int i = 0; i < count; ++i) {
            col = 2;
            mvprintw(start_row + 2 + i, col, "%-3d ", list[i].id);
            col += 4;
            mvprintw(start_row + 2 + i, col, "%-9d ", list[i].student_number);
            col += 10;
            mvprintw(start_row + 2 + i, col, "%-14s ", list[i].name);
            col += 15;
            for (int j = 0; j < NUM_SUBJECTS; ++j) {
                char grade_info[20];
                sprintf(grade_info, "%3d (%c)", list[i].grades[j], list[i].letter_grades[j]);
                mvprintw(start_row + 2 + i, col, "%-15s ", grade_info);
                col += 16;
            }
            mvprintw(start_row + 2 + i, col, "%-6d %-7.2f", list[i].total_score, list[i].average);
        }
    }

    // Instructions
    attron(A_DIM);
    mvprintw(rows - 4, 2, "Use arrow keys to navigate options below.");
    attroff(A_DIM);

    // Options
    char *options[] = {
            "1. Go Back",
            "2. Return to Main Menu"
    };
    int n_options = sizeof(options) / sizeof(char *);
    int highlight = 0;
    int c;
    int choice = -1;
    while(1) {
        choice = -1; // Reset choice
        // Display options
        for (int i = 0; i < n_options; ++i) {
            if (i == highlight) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(rows - 3 + i, 2, "%s", options[i]);
                attroff(A_REVERSE | A_BOLD);
            } else {
                mvprintw(rows - 3 + i, 2, "%s", options[i]);
            }
        }
        c = getch();
        switch(c) {
            case KEY_UP:
                highlight = (highlight - 1 + n_options) % n_options;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_options;
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                break;
        }
        if (choice != -1) {
            if (choice == 0) { // Go Back
                return; // Return to the previous menu
            } else { // Return to Main Menu
                longjmp(mainMenuJmpBuf, 1); // Return to main menu
            }
        }
    }
}

void sortedOutput() {
    int sort_order = 1; // 1 for ascending, -1 for descending
    int highlight = 1;
    int choice = -1;
    int c;
    char *choices[] = {
            "Sort Order: Ascending",
            "1. Sort by Name",
            "2. Sort by Student Number",
            "3. Sort by Total Score",
            "4. Go Back"
    };
    int n_choices = sizeof(choices) / sizeof(char *);
    while(1) {
        choice = -1; // Reset choice
        clear();
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        // Draw border
        box(stdscr, 0, 0);

        // Title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(1, (cols - strlen("Sorted Output"))/2, "Sorted Output");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(2, 1, ACS_HLINE, cols - 2);

        // Update sort order display
        if (sort_order == 1)
            choices[0] = "Sort Order: Ascending";
        else
            choices[0] = "Sort Order: Descending";

        // Menu
        int start_row = 4;
        for (int i = 0; i < n_choices; ++i) {
            if (i == highlight) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(start_row + i, 2, "%s", choices[i]);
                attroff(A_REVERSE | A_BOLD);
            } else {
                mvprintw(start_row + i, 2, "%s", choices[i]);
            }
        }

        // Instructions
        attron(A_DIM);
        mvprintw(rows - 2, 2, "Use arrow keys to navigate, Enter to select.");
        attroff(A_DIM);

        c = getch();
        switch(c) {
            case KEY_UP:
                highlight = (highlight - 1 + n_choices) % n_choices;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_choices;
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                break;
        }
        if (choice != -1) {
            if (choice == 0) {
                sort_order *= -1; // Toggle sort order
            } else if (choice == 1) {
                // Sort by name
                sortStudentsByName(sort_order);
                displayStudents(students, num_students, 2); // return_code = 2 (Return to Sorted Output)
            } else if (choice == 2) {
                // Sort by student number
                sortStudentsByNumber(sort_order);
                displayStudents(students, num_students, 2);
            } else if (choice == 3) {
                // Sort by total score
                sortStudentsByTotalScore(sort_order);
                displayStudents(students, num_students, 2);
            } else if (choice == 4) {
                // Go back to View Students menu
                return;
            }
        }
    }
}

void sortStudentsByName(int order) {
    // Simple bubble sort
    for (int i = 0; i < num_students -1; ++i) {
        for (int j = 0; j < num_students - i -1; ++j) {
            if (order * strcmp(students[j].name, students[j+1].name) > 0) {
                // Swap
                Student temp = students[j];
                students[j] = students[j+1];
                students[j+1] = temp;
            }
        }
    }
}

void sortStudentsByNumber(int order) {
    for (int i = 0; i < num_students -1; ++i) {
        for (int j = 0; j < num_students - i -1; ++j) {
            if (order * (students[j].student_number - students[j+1].student_number) > 0) {
                Student temp = students[j];
                students[j] = students[j+1];
                students[j+1] = temp;
            }
        }
    }
}

void sortStudentsByTotalScore(int order) {
    for (int i = 0; i < num_students -1; ++i) {
        for (int j = 0; j < num_students - i -1; ++j) {
            if (order * (students[j].total_score - students[j+1].total_score) > 0) {
                Student temp = students[j];
                students[j] = students[j+1];
                students[j+1] = temp;
            }
        }
    }
}

void searchOutput() {
    while(1) {
        echo();
        curs_set(1); // Show cursor
        char search_name[50];
        clear();
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        // Draw border
        box(stdscr, 0, 0);

        // Title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(1, (cols - strlen("Search Student by Name"))/2, "Search Student by Name");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(2, 1, ACS_HLINE, cols - 2);

        mvprintw(4, 2, "Enter Name: ");
        move(4, 14);
        clrtoeol();
        refresh();
        getnstr(search_name, sizeof(search_name) - 1);

        noecho();
        curs_set(0); // Hide cursor

        // Search for students with matching name
        Student found_students[MAX_STUDENTS];
        int found_count = 0;
        for (int i = 0; i < num_students; ++i) {
            if (strcmp(students[i].name, search_name) == 0) {
                found_students[found_count++] = students[i];
            }
        }

        // Display results
        clear();
        // Draw border
        box(stdscr, 0, 0);

        // Title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(1, (cols - strlen("Search Results"))/2, "Search Results");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(2, 1, ACS_HLINE, cols - 2);

        int choice = -1;
        if (found_count > 0) {
            displayStudents(found_students, found_count, 0); // return_code = 0
            // After displaying, prompt to go back
            return; // Return to View Students
        } else {
            mvprintw(4, 2, "No students found with that name.");
            // Options
            char *choices[] = {
                    "1. Search Again",
                    "2. Return to View Students",
                    "3. Return to Menu"
            };
            int n_choices = sizeof(choices) / sizeof(char *);
            int highlight = 0;
            int c;
            while(1) {
                choice = -1; // Reset choice
                // Display options
                for (int i = 0; i < n_choices; ++i) {
                    if (i == highlight) {
                        attron(A_REVERSE | A_BOLD);
                        mvprintw(6 + i, 2, "%s", choices[i]);
                        attroff(A_REVERSE | A_BOLD);
                    } else {
                        mvprintw(6 + i, 2, "%s", choices[i]);
                    }
                }
                c = getch();
                switch(c) {
                    case KEY_UP:
                        highlight = (highlight - 1 + n_choices) % n_choices;
                        break;
                    case KEY_DOWN:
                        highlight = (highlight + 1) % n_choices;
                        break;
                    case 10: // Enter key
                        choice = highlight;
                        break;
                    default:
                        break;
                }
                if (choice != -1)
                    break;
            }
            if (choice == 0) {
                continue; // Search again
            } else if (choice == 1) {
                return; // Return to View Students
            } else if (choice == 2) {
                longjmp(mainMenuJmpBuf, 1); // Return to main menu
            }
        }
    }
}


void modifyStudentInfo() {
    if (num_students == 0) {
        clear();
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        // Draw border
        box(stdscr, 0, 0);
        mvprintw(rows / 2, (cols - strlen("No registered students.")) / 2, "No registered students.");
        mvprintw(rows - 2, 2, "Press Enter to return to the menu.");
        getch();
        return;
    }
    int highlight = 0;
    int c;
    while(1) {
        clear();
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        // Draw border
        box(stdscr, 0, 0);

        // Title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(1, (cols - strlen("Modify Student Info"))/2, "Modify Student Info");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(2, 1, ACS_HLINE, cols - 2);

        // Student list
        int start_row = 4;
        for (int i = 0; i < num_students; ++i) { // Changed loop to start from 0
            if (i == highlight) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(start_row + i, 2, "%d. %s", students[i].id, students[i].name);
                attroff(A_REVERSE | A_BOLD);
            } else {
                mvprintw(start_row + i, 2, "%d. %s", students[i].id, students[i].name);
            }
        }

        // Instructions
        attron(A_DIM);
        mvprintw(rows - 2, 2, "Use arrow keys to select, Enter to modify, 'q' to quit.");
        attroff(A_DIM);

        c = getch();
        if (c == 'q' || c == 'Q') {
            break;
        } else if (c == KEY_UP) {
            highlight = (highlight - 1 + num_students) % num_students;
        } else if (c == KEY_DOWN) {
            highlight = (highlight + 1) % num_students;
        } else if (c == 10) { // Enter key
            editStudent(&students[highlight]);
        }
    }
}

void editStudent(Student *s) {
    echo();
    curs_set(1);
    clear();
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Draw border
    box(stdscr, 0, 0);

    // Title
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(1, (cols - strlen("Edit Student Information"))/2, "Edit Student Information");
    attroff(COLOR_PAIR(2) | A_BOLD);

    mvhline(2, 1, ACS_HLINE, cols - 2);

    int start_row = 4;

    mvprintw(start_row, 2, "Current Student Number: %d", s->student_number);
    s->student_number = getIntegerInput("New Student Number: ");

    mvprintw(start_row + 2, 2, "Current Name: %s", s->name);
    getStringInput("New Name: ", s->name, sizeof(s->name));

    s->total_score = 0;
    for (int i = 0; i < NUM_SUBJECTS; ++i) {
        mvprintw(start_row + 4 + i*2, 2, "Current %s Grade: %d (%c)", subject_names[i], s->grades[i], s->letter_grades[i]);
        char prompt[50];
        sprintf(prompt, "New %s Grade: ", subject_names[i]);
        s->grades[i] = getIntegerInput(prompt);
        s->total_score += s->grades[i];
        s->letter_grades[i] = assignLetterGrade(s->grades[i]);
    }
    s->average = s->total_score / (double)NUM_SUBJECTS;

    mvprintw(start_row + NUM_SUBJECTS * 2 + 6, 2, "Modification completed!");
    attron(A_DIM);
    mvprintw(rows - 2, 2, "Press Enter to return to the menu.");
    attroff(A_DIM);
    getch();
    noecho();
    curs_set(0);
}

void deleteStudent() {
    if (num_students == 0) {
        clear();
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        // Draw border
        box(stdscr, 0, 0);
        mvprintw(rows / 2, (cols - strlen("No registered students.")) / 2, "No registered students.");
        mvprintw(rows - 2, 2, "Press Enter to return to the menu.");
        getch();
        return;
    }
    int highlight = 0;
    int c;
    while(1) {
        clear();
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        // Draw border
        box(stdscr, 0, 0);

        // Title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(1, (cols - strlen("Delete Student"))/2, "Delete Student");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(2, 1, ACS_HLINE, cols - 2);

        // Student list
        int start_row = 4;
        for (int i = 0; i < num_students; ++i) { // Changed loop to start from 0
            if (i == highlight) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(start_row + i, 2, "%d. %s", students[i].id, students[i].name);
                attroff(A_REVERSE | A_BOLD);
            } else {
                mvprintw(start_row + i, 2, "%d. %s", students[i].id, students[i].name);
            }
        }

        // Instructions
        attron(A_DIM);
        mvprintw(rows - 2, 2, "Use arrow keys to select, Enter to delete, 'q' to quit.");
        attroff(A_DIM);

        c = getch();
        if (c == 'q' || c == 'Q') {
            break;
        } else if (c == KEY_UP) {
            highlight = (highlight - 1 + num_students) % num_students;
        } else if (c == KEY_DOWN) {
            highlight = (highlight + 1) % num_students;
        } else if (c == 10) { // Enter key
            // Confirm deletion
            clear();
            // Draw border
            box(stdscr, 0, 0);
            mvprintw(1, (cols - strlen("Delete Student"))/2, "Delete Student");
            mvprintw(rows / 2, (cols - strlen("Are you sure you want to delete? (y/n)")) / 2, "Are you sure you want to delete? (y/n)");
            c = getch();
            if (c == 'y' || c == 'Y') {
                // Delete student
                for (int i = highlight; i < num_students - 1; ++i) {
                    students[i] = students[i + 1];
                }
                num_students--;
                mvprintw(rows / 2 + 1, (cols - strlen("Deletion completed!")) / 2, "Deletion completed!");
                mvprintw(rows - 2, 2, "Press Enter to return to the menu.");
                getch();
                break;
            } else {
                break;
            }
        }
    }
}

int getIntegerInput(const char *prompt) {
    char input[50];
    int value;
    while (1) {
        mvprintw(getcury(stdscr), 2, "%s", prompt);
        move(getcury(stdscr), strlen(prompt) + 2);
        clrtoeol();
        refresh();
        getnstr(input, sizeof(input) - 1);
        int valid = 1;
        for (int i = 0; input[i] != '\0'; ++i) {
            if (!isdigit(input[i]) && !(i == 0 && input[i] == '-')) {
                valid = 0;
                break;
            }
        }
        if (valid) {
            value = atoi(input);
            break;
        } else {
            mvprintw(getcury(stdscr) + 1, 2, "Invalid input. Please enter a valid integer.");
            move(getcury(stdscr) + 1, 0);
            clrtoeol();
        }
    }
    return value;
}

void getStringInput(const char *prompt, char *buffer, int buffer_size) {
    mvprintw(getcury(stdscr), 2, "%s", prompt);
    move(getcury(stdscr), strlen(prompt) + 2);
    clrtoeol();
    refresh();
    getnstr(buffer, buffer_size - 1);
}