#include "string.h"

void atoi(int value, char* str, int base) {
	if (base < 2 || base > 36) {
		*str = '\0';
	}
	char* letter = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int sign = value, i = 0;
	unsigned int v = (unsigned int)value;
	if (sign < 0 && base == 10) v = (unsigned int)-value;
	do {
		str[i++] = letter[v % base];
	} while ((v /= base) > 0);
	if (sign < 0 && base == 10) str[i++] = '-';
	str[i++] = '\0';
	reverse(str);
}

void reverse(char str[]) {
	int i = 0, j = strlen(str) - 1;
	while (i < j) {
		char c = str[i];
		str[i] = str[j];
		str[j] = c;
		i++;
		j--;
	}
}

int strlen(char str[]) {
	int i = 0;
	while (str[i] != '\0') ++i;
	return i;
}

void backspace(char str[]) {
	int len = strlen(str);
	str[len - 1] = '\0';
}

void append(char str[], char n) {
	int len = strlen(str);
	str[len] = n;
	str[len + 1] = '\0';
}

int strcmp(char str1[], char str2[]) {
	int i;
	for (i = 0; str1[i] == str2[i]; ++i) {
		if (str1[i] == '\0') return 0;
	}
	return str1[i] - str2[i];
}
