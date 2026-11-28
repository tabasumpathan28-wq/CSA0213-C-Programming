/* Blood Bank Inventory and Emergency Donor Matching System
   CSA0213 - C Programming | C99
   Compile: gcc blood_bank.c -o blood_bank
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 200
#define TEXT 60

struct Date {
    int day, month, year;
};

struct BloodUnit {
    char group[4];
    int quantity;
    struct Date collection;
    struct Date expiry;
};

struct Donor {
    char name[TEXT];
    char group[4];
    char phone[20];
    struct Date lastDonation;
};

struct Request {
    char hospital[TEXT];
    char patient[TEXT];
    char group[4];
    int units;
    struct Date date;
};

struct Donation {
    char donorName[TEXT];
    char group[4];
    int units;
    struct Date date;
};

struct BloodUnit stock[MAX];
struct Donor donors[MAX];
struct Request requests[MAX];
struct Donation history[MAX];
int stockCount = 0, donorCount = 0, requestCount = 0, historyCount = 0;

const char *groups[] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"};

void readLine(const char *prompt, char *out, int size) {
    int ch;
    printf("%s", prompt);
    if (fgets(out, size, stdin) == NULL) {
        out[0] = '\0';
        return;
    }
    out[strcspn(out, "\n")] = '\0';
    if (strchr(out, '\n') == NULL) {
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
}

int readInt(const char *prompt) {
    char line[40], extra;
    int value;
    while (1) {
        readLine(prompt, line, sizeof(line));
        if (sscanf(line, "%d %c", &value, &extra) == 1) return value;
        printf("Please enter a valid number.\n");
    }
}

struct Date readDate(const char *prompt) {
    struct Date d;
    char line[50], extra;
    while (1) {
        readLine(prompt, line, sizeof(line));
        if (sscanf(line, "%d/%d/%d %c", &d.day, &d.month, &d.year, &extra) == 3 &&
            d.day >= 1 && d.day <= 31 && d.month >= 1 && d.month <= 12 &&
            d.year >= 2000 && d.year <= 2100) return d;
        printf("Enter date in DD/MM/YYYY format.\n");
    }
}

void dateToString(struct Date d, char *out, size_t size) {
    snprintf(out, size, "%02d/%02d/%04d", d.day, d.month, d.year);
}

int compareDate(struct Date a, struct Date b) {
    if (a.year != b.year) return a.year - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.day - b.day;
}

void upper(char *s) {
    int i;
    for (i = 0; s[i]; i++) s[i] = (char)toupper((unsigned char)s[i]);
}

int validGroup(const char *g) {
    int i;
    for (i = 0; i < 8; i++)
        if (strcmp(g, groups[i]) == 0) return 1;
    return 0;
}

void readGroup(char *g) {
    while (1) {
        readLine("Blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ", g, 4);
        upper(g);
        if (validGroup(g)) return;
        printf("Invalid blood group. Try again.\n");
    }
}

void saveData(void) {
    FILE *fp;
    fp = fopen("stock.dat", "wb");
    if (fp) { fwrite(&stockCount, sizeof(int), 1, fp); fwrite(stock, sizeof(struct BloodUnit), stockCount, fp); fclose(fp); }
    fp = fopen("donors.dat", "wb");
    if (fp) { fwrite(&donorCount, sizeof(int), 1, fp); fwrite(donors, sizeof(struct Donor), donorCount, fp); fclose(fp); }
    fp = fopen("requests.dat", "wb");
    if (fp) { fwrite(&requestCount, sizeof(int), 1, fp); fwrite(requests, sizeof(struct Request), requestCount, fp); fclose(fp); }
    fp = fopen("history.dat", "wb");
    if (fp) { fwrite(&historyCount, sizeof(int), 1, fp); fwrite(history, sizeof(struct Donation), historyCount, fp); fclose(fp); }
}

void loadOne(const char *filename, void *array, int *count, size_t itemSize) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return;
    if (fread(count, sizeof(int), 1, fp) != 1 || *count < 0 || *count > MAX) {
        *count = 0;
    } else {
        fread(array, itemSize, (size_t)*count, fp);
    }
    fclose(fp);
}

void loadData(void) {
    loadOne("stock.dat", stock, &stockCount, sizeof(struct BloodUnit));
    loadOne("donors.dat", donors, &donorCount, sizeof(struct Donor));
    loadOne("requests.dat", requests, &requestCount, sizeof(struct Request));
    loadOne("history.dat", history, &historyCount, sizeof(struct Donation));
}

void addStock(void) {
    struct BloodUnit b;
    if (stockCount >= MAX) { printf("Stock record limit reached.\n"); return; }
    readGroup(b.group);
    b.quantity = readInt("Number of units: ");
    if (b.quantity <= 0) { printf("Quantity must be positive.\n"); return; }
    b.collection = readDate("Collection date (DD/MM/YYYY): ");
    b.expiry = readDate("Expiry date (DD/MM/YYYY): ");
    if (compareDate(b.expiry, b.collection) <= 0) {
        printf("Expiry date must be after collection date.\n"); return;
    }
    stock[stockCount++] = b;
    saveData();
    printf("Blood stock added successfully.\n");
}

void sortStockByExpiry(void) {
    int i, j;
    struct BloodUnit temp;
    for (i = 0; i < stockCount - 1; i++)
        for (j = 0; j < stockCount - i - 1; j++)
            if (compareDate(stock[j].expiry, stock[j + 1].expiry) > 0) {
                temp = stock[j]; stock[j] = stock[j + 1]; stock[j + 1] = temp;
            }
}

void viewStock(void) {
    int i, j, total;
    char c[20], e[20];
    sortStockByExpiry();
    printf("\n--- BLOOD STOCK (earliest expiry first) ---\n");
    if (stockCount == 0) printf("No stock records available.\n");
    for (i = 0; i < stockCount; i++) {
        dateToString(stock[i].collection, c, sizeof(c));
        dateToString(stock[i].expiry, e, sizeof(e));
        printf("%2d. %-3s | Units: %-4d | Collected: %s | Expires: %s%s\n",
               i + 1, stock[i].group, stock[i].quantity, c, e,
               stock[i].quantity > 0 ? "" : " | OUT OF STOCK");
    }
    printf("\n--- SUMMARY BY BLOOD GROUP ---\n");
    for (i = 0; i < 8; i++) {
        total = 0;
        for (j = 0; j < stockCount; j++)
            if (strcmp(stock[j].group, groups[i]) == 0) total += stock[j].quantity;
        printf("%-3s : %d units\n", groups[i], total);
    }
}

int compatible(const char *patient, const char *donor) {
    if (strcmp(patient, "O-") == 0) return strcmp(donor, "O-") == 0;
    if (strcmp(patient, "O+") == 0) return strcmp(donor, "O-") == 0 || strcmp(donor, "O+") == 0;
    if (strcmp(patient, "A-") == 0) return strcmp(donor, "O-") == 0 || strcmp(donor, "A-") == 0;
    if (strcmp(patient, "A+") == 0)
        return strcmp(donor, "O-") == 0 || strcmp(donor, "O+") == 0 || strcmp(donor, "A-") == 0 || strcmp(donor, "A+") == 0;
    if (strcmp(patient, "B-") == 0) return strcmp(donor, "O-") == 0 || strcmp(donor, "B-") == 0;
    if (strcmp(patient, "B+") == 0)
        return strcmp(donor, "O-") == 0 || strcmp(donor, "O+") == 0 || strcmp(donor, "B-") == 0 || strcmp(donor, "B+") == 0;
    if (strcmp(patient, "AB-") == 0)
        return strcmp(donor, "O-") == 0 || strcmp(donor, "A-") == 0 || strcmp(donor, "B-") == 0 || strcmp(donor, "AB-") == 0;
    if (strcmp(patient, "AB+") == 0) return 1;
    return 0;
}

void registerDonor(void) {
    struct Donor d;
    if (donorCount >= MAX) { printf("Donor record limit reached.\n"); return; }
    readLine("Donor name: ", d.name, sizeof(d.name));
    readGroup(d.group);
    readLine("Phone number: ", d.phone, sizeof(d.phone));
    d.lastDonation = readDate("Last donation date (DD/MM/YYYY): ");
    donors[donorCount++] = d;
    saveData();
    printf("Donor registered successfully.\n");
}

void searchDonors(void) {
    char patientGroup[4];
    int i, found = 0;
    readGroup(patientGroup);
    printf("\nCompatible registered donors for %s:\n", patientGroup);
    for (i = 0; i < donorCount; i++) {
        if (compatible(patientGroup, donors[i].group)) {
            printf("%s | Group: %s | Phone: %s\n", donors[i].name, donors[i].group, donors[i].phone);
            found = 1;
        }
    }
    if (!found) printf("No compatible registered donors found.\n");
}

void recordRequest(void) {
    struct Request r;
    int i, remaining, issued = 0;
    if (requestCount >= MAX) { printf("Request record limit reached.\n"); return; }
    readLine("Hospital name: ", r.hospital, sizeof(r.hospital));
    readLine("Patient name: ", r.patient, sizeof(r.patient));
    readGroup(r.group);
    r.units = readInt("Units required: ");
    if (r.units <= 0) { printf("Units must be positive.\n"); return; }
    r.date = readDate("Request date (DD/MM/YYYY): ");

    remaining = r.units;
    sortStockByExpiry();
    for (i = 0; i < stockCount && remaining > 0; i++) {
        if (strcmp(stock[i].group, r.group) == 0 && stock[i].quantity > 0) {
            int take = stock[i].quantity < remaining ? stock[i].quantity : remaining;
            stock[i].quantity -= take;
            remaining -= take;
            issued += take;
        }
    }
    requests[requestCount++] = r;
    saveData();
    printf("Request recorded. Units issued: %d", issued);
    if (remaining > 0) printf(" | Units not available: %d", remaining);
    printf("\n");
}

void recordDonation(void) {
    struct Donation d;
    int i, found = 0;
    if (historyCount >= MAX) { printf("History record limit reached.\n"); return; }
    readLine("Donor name: ", d.donorName, sizeof(d.donorName));
    readGroup(d.group);
    d.units = readInt("Units donated: ");
    if (d.units <= 0) { printf("Units must be positive.\n"); return; }
    d.date = readDate("Donation date (DD/MM/YYYY): ");
    history[historyCount++] = d;

    /* Add the donation to stock as a new stock batch. */
    if (stockCount < MAX) {
        strcpy(stock[stockCount].group, d.group);
        stock[stockCount].quantity = d.units;
        stock[stockCount].collection = d.date;
        stock[stockCount].expiry.day = d.date.day;
        stock[stockCount].expiry.month = d.date.month;
        stock[stockCount].expiry.year = d.date.year + 1; /* illustrative shelf life */
        stockCount++;
        found = 1;
    }
    saveData();
    printf("Donation saved to history%s.\n", found ? " and added to stock" : "");
}

void viewHistory(void) {
    int i;
    char date[20];
    printf("\n--- DONATION HISTORY ---\n");
    if (historyCount == 0) printf("No donation history available.\n");
    for (i = 0; i < historyCount; i++) {
        dateToString(history[i].date, date, sizeof(date));
        printf("%d. %s | %s | %d unit(s) | %s\n", i + 1,
               history[i].donorName, history[i].group, history[i].units, date);
    }
    printf("\n--- HOSPITAL REQUEST HISTORY ---\n");
    if (requestCount == 0) printf("No requests recorded.\n");
    for (i = 0; i < requestCount; i++) {
        dateToString(requests[i].date, date, sizeof(date));
        printf("%d. %s | Patient: %s | %s | %d unit(s) | %s\n",
               i + 1, requests[i].hospital, requests[i].patient,
               requests[i].group, requests[i].units, date);
    }
}

void showMenu(void) {
    printf("\n========== BLOOD BANK MANAGEMENT ==========\n");
    printf("1. Add blood stock\n");
    printf("2. View stock summary (expiry order)\n");
    printf("3. Register donor\n");
    printf("4. Search compatible donors\n");
    printf("5. Record hospital request / issue stock\n");
    printf("6. Record donation\n");
    printf("7. View requests and donation history\n");
    printf("0. Save and exit\n");
}

int main(void) {
    int choice;
    loadData();
    printf("Blood Bank Inventory and Emergency Donor Matching System\n");
    do {
        showMenu();
        choice = readInt("Enter your choice: ");
        switch (choice) {
            case 1: addStock(); break;
            case 2: viewStock(); break;
            case 3: registerDonor(); break;
            case 4: searchDonors(); break;
            case 5: recordRequest(); break;
            case 6: recordDonation(); break;
            case 7: viewHistory(); break;
            case 0: saveData(); printf("Data saved. Thank you!\n"); break;
            default: printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);
    return 0;
}
