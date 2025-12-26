#include <stdio.h>
#include <stdlib.h>

char *convert_to_binary(unsigned int num, unsigned int binary_length)
{
    unsigned int num_bits = 0; /* Count the number of bits */
    char *binary = NULL;       /* Initialize binary pointer to NULL */
    unsigned int temp = num;   /* Initialize temp to num */
    int i = 0;

    if (num == 0)
        return "0";

    if (binary_length > 0)
        num_bits = binary_length;
    else
    {
        /* Calculate the number of bits needed */
        while (temp > 0)
        {
            temp >>= 1; /* Right shift to divide by 2 */
            num_bits++;
        }
    }

    binary = (char *)malloc((num_bits + 1) * sizeof(char)); /* +1 for null terminator */
    if (binary == NULL)
        return NULL;         /* Memory allocation failed */
    binary[num_bits] = '\0'; /* Null terminate the string */

    for (i = num_bits - 1; i >= 0; i--)
    {
        binary[i] = (num & 1) ? '1' : '0'; /* Check the least significant bit */
        num >>= 1;                         /* Right shift to process the next bit */
    }

    return binary;
}
