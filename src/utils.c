#include <stdio.h>
#include <stdlib.h>

int convert_to_binary(unsigned int number)
{
    /* Calculate total number of bits in an unsigned int (around 32 bits) */
    int num_bits = sizeof(unsigned int) * 8;
    char *buffer = malloc(num_bits + 1); /* Buffer to hold binary representation string + null terminator */
    unsigned int i;                      /* Loop counter variable*/
    unsigned int mask;                   /* Bit mask used to isolate and test individual bits in a number*/

    if (buffer == NULL)
        return -1; /* Memory allocation failed */

    /* Loop through each bit position */
    /* Start from the leftmost bit (highest position) and move right (lowest position) */
    for (i = 0; i < num_bits; i++)
    {
        /* Create a mask with a 1 bit at position num_of_bits - 1 - i (from left to right) */
        /* Example: if i=3, mask = 00001000 in binary */
        mask = 1 << (num_bits - 1 - i);
        /* Perform bitwise AND between number and mask */
        /* If the bit at position i is 1, set 1; otherwise set 0 */
        buffer[i] = (number & mask) ? '1' : '0';
    }
    /* Null terminate the string */
    buffer[num_bits] = '\0';
    return buffer;
}
