/*
 * File: node.c
 * Description: Generic linked list implementation.
 * Used for storing macros (Pre-Assembler) and external references (Second Pass).
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/node.h"
#include "../headers/utils.h"
#include "../headers/error.h"

/**
 * Function: create_node
 * Description: Creates a new linked list node.
 * Allocates memory for the node structure and copies the strings.
 * @param key - The identifier or name for this node (e.g., macro name, label name)
 * @param content - The data or value associated with this key (e.g., macro content, address)
 * @return Pointer to the newly created node, or NULL if memory allocation fails
 */

node *create_node(char *key, char *content)
{
    /* Allocate memory for the node structure itself */
    node *temp = (node *)handle_malloc(sizeof(node));

    /* Handle NULL parameters by replacing them with empty strings */
    /* This prevents segmentation faults when copying strings later */
    if (!key)
        key = "";
    if (!content)
        content = "";

    /* Allocate memory for the key string (+1 for null terminator) */
    temp->key = (char *)malloc(strlen(key) + 1);
    /* Allocate memory for the content string (+1 for null terminator) */
    temp->content = (char *)malloc(strlen(content) + 1);

    /* Check if either allocation failed */
    if (!(temp->key) || !(temp->content))
    {
        /* Log error and clean up any allocated memory before returning */
        log_error(ERR_OPEN_FILE, 0, NULL, NULL);
        free_node(temp);
        return NULL;
    }
    /* Copy the strings into the newly allocated memory */
    strcpy(temp->key, key);
    strcpy(temp->content, content);

    /* Initialize next pointer to NULL (end of list) */
    temp->next = NULL;
    return temp;
}

/**
 * Function: add_node
 * Description: Appends a node to the end of the list
 * @param head - Pointer to the first node in the linked list
 * @param new_node - The node to be added at the end of the list
 */
void add_node(node *head, node *new_node)
{
    node *current = head;

    /* Validate input parameters to prevent null pointer dereferencing */
    if (!head || !new_node)
        return;

    /* Traverse the list until we reach the last node */
    /* The last node is identified by having next == NULL */
    while (current->next != NULL)
        current = current->next;

    /* Link the new node to the end of the list */
    current->next = new_node;
}

/**
 * Function: search_node
 * Description: Searches for a node by key string
 * @param head - Pointer to the first node in the linked list
 * @param key - The key string to search for
 * @return Pointer to the found node, or NULL if not found
 */
node *search_node(node *head, char *key)
{
    node *current = head;

    /* Traverse the entire list */
    while (current != NULL)
    {
        /* Compare the current node's key with the search key */
        if (strcmp(current->key, key) == 0)
            return current;
        current = current->next; /* Move to the next node in the list */
    }
    return NULL; /* Key not found in the list */
}

/**
 * Function: free_node
 * Description: Frees memory allocated for a single node
 * @param n - Pointer to the node to be freed
 */
void free_node(node *n)
{
    /* Check if the node pointer is valid */
    if (!n)
        return;
    /* Free the dynamically allocated key string */
    if (n->key)
        free(n->key);
    /* Free the dynamically allocated content string */
    if (n->content)
        free(n->content);
    /* Finally, free the node structure itself */
    free(n);
}

/**
 * Function: free_nodes
 * Description: Frees an entire linked list
 * @param head - Pointer to the first node in the list
 */
void free_nodes(node *head)
{
    node *temp;

    /* Iterate through the list until all nodes are freed */
    while (head != NULL)
    {
        /* Save the current node in a temporary variable */
        temp = head;
        /* Move head pointer to the next node before freeing */
        head = head->next;
        /* Free the current node (including its strings) */
        free_node(temp);
    }
}