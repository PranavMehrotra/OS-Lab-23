#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char ***parse_line(char *line) {
    int i, j, k;
    int len = strlen(line);
    int in_single_quote = 0, in_double_quote = 0, backslash = 0;
    int num_pipes = 0, num_args = 0;
    int start, end,tem_start;
    char ***commands;

    // Count the number of pipes in the line
    for (i = 0; i < len; i++) {
        if (line[i] == '\'' && !in_double_quote && !backslash)
            in_single_quote = !in_single_quote;
        else if (line[i] == '\"' && !in_single_quote && !backslash)
            in_double_quote = !in_double_quote;
        else if (line[i] == '\\' && !backslash)
            backslash = 1;
        else if (line[i] == '|' && !in_single_quote && !in_double_quote && !backslash)
            num_pipes++;
        else
            backslash = 0;
    }

    // Allocate memory for the commands array
    commands = (char ***) malloc((num_pipes + 2) * sizeof(char **));
    commands[num_pipes+1] = NULL;
    // Split the line by pipes
    start = 0;
    end = 0;
    for (i = 0; i <= num_pipes; i++) {
        // Find the end of the current command
        in_single_quote = 0, in_double_quote = 0, backslash = 0;
        num_args = 0;
        for (end = start; end < len; end++) {
            if (line[end] == '\'' && !in_double_quote && !backslash)
                in_single_quote = !in_single_quote;
            else if (line[end] == '\"' && !in_single_quote && !backslash)
                in_double_quote = !in_double_quote;
            else if (line[end] == '\\' && !backslash)
                backslash = 1;
            else if (line[end] == '|' && !in_single_quote && !in_double_quote && !backslash)
                break;
            else
                backslash = 0;
        }
        if(line[start]==' ')    start++;
        tem_start = start;
        // Count the number of arguments in the current command
        for (j = start; j < end; j++) {
            if (line[j] == '\'' && !in_double_quote && !backslash)
                in_single_quote = !in_single_quote;
            else if (line[j] == '\"' && !in_single_quote && !backslash)
                in_double_quote = !in_double_quote;
            else if (line[j] == '\\' && !backslash)
                backslash = 1;
            else if (line[j] == ' ' && !in_single_quote && !in_double_quote && !backslash)
                num_args++;
            else
                backslash = 0;
        }
        if(line[j-1]!=' ')
            num_args++;
        // Allocate memory for the argument array for the current command
        commands[i] = (char **) malloc((num_args + 1) * sizeof(char *));

        // Split the current command into its individual arguments and flags
        start = tem_start;
        in_single_quote = 0, in_double_quote = 0, backslash = 0;
        j = start;
        for (k = 0; k < num_args; k++) {
            // Find the end of the current argument
            for (; j < end; j++) {
                if (line[j] == '\'' && !in_double_quote && !backslash)
                    in_single_quote = !in_single_quote;
                else if (line[j] == '\"' && !in_single_quote && !backslash)
                    in_double_quote = !in_double_quote;
                else if (line[j] == '\\' && !backslash)
                    backslash = 1;
                else if (line[j] == ' ' && !in_single_quote && !in_double_quote && !backslash)
                    break;
                else
                    backslash = 0;
            }

            // Allocate memory for the current argument
            commands[i][k] = (char *) malloc((j - start + 1) * sizeof(char));

            // Copy the current argument into the argument array
            strncpy(commands[i][k], line + start, j - start);
            commands[i][k][j - start] = '\0';

            // Move on to the next argument
            start = j + 1;
            j = start;
        }

        // Add a NULL entry at the end of the argument array
        commands[i][num_args] = NULL;

        // Move on to the next command
        start = end + 1;
        end = start;
    }

    return commands;
}

int main(){
    char *line = "ls -l | grep 'hello world' | wc -l";
    char ***commands = parse_line(line);
    int i, j;

    for (i = 0; commands[i] != NULL; i++) {
        for (j = 0; commands[i][j] != NULL; j++)
            printf("%s ", commands[i][j]);
        printf("\n");
    }
}