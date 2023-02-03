#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *** parse_line(char * line) {
    int i, j, k;
    int cmd_count = 0;
    int cmd_len = 0;
    int arg_count = 0;
    int arg_len = 0;
    int in_quote = 0;
    int in_double_quote = 0;
    int escape = 0;
    char *** cmds;
    char ** args;
    char * arg;

    // Count the number of commands in the line
    for (i = 0; line[i]; i++) {
        if (line[i] == '|' && !in_quote && !in_double_quote && !escape) {
            cmd_count++;
        } else if (line[i] == '\'' && !in_double_quote && !escape) {
            in_quote = !in_quote;
        } else if (line[i] == '\"' && !in_quote && !escape) {
            in_double_quote = !in_double_quote;
        } else if (line[i] == '\\' && !escape) {
            escape = 1;
        } else {
            escape = 0;
        }
    }

    // Allocate memory for the list of commands
    cmds = malloc((cmd_count + 1) * sizeof(char **));
    if (!cmds) {
        return NULL;
    }
    cmds[cmd_count] = NULL;

    // Parse each command in the line
    cmd_len = 0;
    for (i = 0; line[i]; i++) {
        if (line[i] == '|' && !in_quote && !in_double_quote && !escape) {
            cmds[cmd_len++] = args;
            args = NULL;
            arg_count = 0;
            arg_len = 0;
        } else if (line[i] == '\'' && !in_double_quote && !escape) {
            in_quote = !in_quote;
        } else if (line[i] == '\"' && !in_quote && !escape) {
            in_double_quote = !in_double_quote;
        } else if (line[i] == '\\' && !escape) {
            escape = 1;
        } else {
            if (!args) {
                args = malloc((arg_count + 1) * sizeof(char *));
                if (!args) {
                    for (j = 0; j < cmd_len; j++) {
                        for (k = 0; cmds[j][k]; k++) {
                            free(cmds[j][k]);
                        }
                        free(cmds[j]);
                    }
                    free(cmds);
                    return NULL;
                }
                args[arg_count] = NULL;
            }

            if (line[i] == ' ' && !in_quote && !in_double_quote && !escape) {
                if (arg_len) {
                    args[arg_count++] = arg;
                    args[arg_
