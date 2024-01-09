#include <stdio.h>
#include <stdlib.h>

void sum_diff(int a, int b, int *sum, int *diff) {
    *sum = a + b; // Calculate sum and store it in the address pointed by sum
    *diff = a - b; // Calculate difference and store it in the address pointed by diff
}

typedef long (*map_fn)(long); // Define a function pointer type

long double_value(long x) {
    return 2 * x;
}

struct Node {
    int data; // Data field to store the value
    struct Node* next; // Pointer to the next node in the list
};

void insertNode(struct Node** head, int newData) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node)); // Dynamically allocate memory for a new node
    newNode->data = newData; // Set the data for the new node
    newNode->next = *head; // Point the next of new node to the current head
    *head = newNode; // Update the head to point to the new node
}

void displayList(struct Node* head) {
    struct Node* current = head; // Start from the head of the list
    while (current != NULL) { // Traverse the list until the end is reached
        printf("%d ", current->data); // Print the data of the current node
        current = current->next; // Move to the next node
    }
    printf("\n"); // Print a newline at the end of the list
}

void map(long *arr, long n, map_fn f) {
    for (long i = 0; i < n; i++) {
        arr[i] = f(arr[i]); // Apply function f to each element
    }
}

int main() {
    int a, b, s, d, numberOfNodes, newNodeData;
    struct Node* head = NULL; // Initialize head to NULL as the list is empty

    // Prompt user for input and read values for sum_diff
    printf("Enter two integers with space in between to calculate their sum and difference: ");
    scanf("%d %d", &a, &b);
    sum_diff(a, b, &s, &d); // Call sum_diff with user provided values
    printf("Sum: %d, Difference: %d\n", s, d); // Output the results

    // Prompt user for the number of nodes and read the value
    printf("Enter the number of nodes: ");
    scanf("%d", &numberOfNodes);
    
    // Loop to read node data and insert nodes into the list
    for (int i = 0; i < numberOfNodes; i++) {
        printf("Enter data for node %d: ", i + 1);
        scanf("%d", &newNodeData); // Read data for the new node
        insertNode(&head, newNodeData); // Insert new node into the list
    }

    // Display the linked list
    printf("The linked list is: ");
    displayList(head); // Call displayList to print the list
    int yg = 0;
    printf("Enter size of array: ");
    scanf("%d", &yg);

    long arr[yg]; // Declare an array of long integers without initializing
    long n = sizeof(arr) / sizeof(arr[0]); // Calculate the number of elements in arr

    // Prompt the user for each element of the array
    for (long i = 0; i < n; i++) {
        printf("Enter element for doubling value %ld: ", i + 1);
        scanf("%ld", &arr[i]); // Use %ld for reading long integers
    }

    // Call map function with the array, its size, and the double_value function
    map(arr, n, double_value);

    // Print the modified array
    for (long i = 0; i < n; i++) {
        printf("%ld ", arr[i]);
        printf(", ");
    }
    printf("\n");

    return 0;
}




