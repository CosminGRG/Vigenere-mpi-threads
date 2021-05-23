#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <execution>

#define ENCRYPT 1
#define DECRYPT 2

bool isSpace(unsigned char c) {
	return (c == ' ');
}

std::string normalizeInput(std::string str)
{
	std::vector<int> spaceIndex;
	str.erase(std::remove_if(str.begin(), str.end(), isSpace), str.end());

	return str;
}

std::string generateKey(std::string str, std::string key)
{
	int counter = 0;
	while (str.size() != key.size())
	{
		if (counter == str.size())
		{
			counter = 0;
		}
		key.push_back(key[counter]);
		counter++;
	}
	return key;
}

char decryptChar(char s, char k)
{
	char c = 0;
	if ((s != '\t') && (!ispunct(s)) && (!isdigit(s)))
	{
		//c = (s - k + 26) % 26;
		//c += 'A';
		c = (((s - 122) - (k - 97)) % 26) + 122;
	}
	else
	{
		c = s;
	}

	return c;
}

char encryptChar(char s, char k)
{
	char c = 0;
	if ((s != '\t') && (!ispunct(s)) && (!isdigit(s)))
	{
		c = (((s - 97) + (k - 97)) % 26) + 97;
	}

	return c;
}

void partialDecrypt(std::vector<char> &result, std::vector<char> str, std::vector<char> key, int start, int stop)
{
	std::string localStr;
	for (int i = start; i < stop && (str[i] != '\0'); i++)
	{
		result.push_back(decryptChar(str[i], key[i]));
	}
}

void partialEncrypt(std::vector<char>& result, std::vector<char> str, std::vector<char> key, int start, int stop)
{
	std::string localStr;
	for (int i = start; i < stop && (str[i] != '\0'); i++)
	{
		result.push_back(encryptChar(str[i], key[i]));
	}
}

void printResult(std::vector<char> result)
{
	for (int i = 0; i < result.size(); i++)
	{
		std::cout << result[i];
	}
	std::cout << std::endl;
}

int main() {
	int option;
	int numOfThreads;

	std::string str;
	std::string key;

	std::vector<int> whitespaces;

	std::cout << "Enter the message (255 chars max): " << std::endl;
	std::getline(std::cin, str);

	std::cout << "Enter the cypher key: " << std::endl;
	std::cin >> key;

	for (int i = 0; i < key.size(); i++)
	{
		if (!isalpha(key[i]))
		{
			std::cout << "Invalid key input";
			exit(-1);
		}
	}

	std::cout << "Choose operation - > 1 to encrypt / 2 to decrypt:" << std::endl;
	std::cin >> option;

	if ((option < 1) || (option > 2))
	{
		std::cout << "Invalid option";
		exit(-1);
	}

	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] == ' ')
		{
			whitespaces.push_back(i);
		}
	}

	std::cout << "How many threads to use: " << std::endl;
	std::cin >> numOfThreads;

	std::cout << "String is: " << str << std::endl;
	std::cout << "Key is: " << key << std::endl;

	key = generateKey(str, key);
	str = normalizeInput(str);

	std::vector<char>strVect(str.begin(), str.end());
	std::vector<char>keyVect(key.begin(), key.end());
	std::vector<std::vector<char>> partialResult(numOfThreads);

	std::cout << "Key after rounding: " << key << std::endl;

	std::cout << "Using " << numOfThreads << " threads" << std::endl;
	std::vector<std::thread> threads;

	// Elements remaining after division among processes
	int remainder = str.size() % numOfThreads;
	// Number of elements per process
	int quotient = str.size() / numOfThreads;

	for (int i = 0; i < numOfThreads; i++)
	{
		int start = i * quotient + std::min(i, remainder);
		int stop = (i + 1) * quotient + std::min(i + 1, remainder);

		if (option == ENCRYPT)
		{
			threads.push_back(std::thread(partialEncrypt, std::ref(partialResult[i]), strVect, keyVect, start, stop));
		}
		else if (option == DECRYPT)
		{
			threads.push_back(std::thread(partialDecrypt, std::ref(partialResult[i]), strVect, keyVect, start, stop));
		}

#ifdef DEBUG
		std::cout << "Thread " << i << " is computing the string from " << start << " to " << stop << std::endl;
#endif
	}

	int i = 0;
	for (std::thread& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}

		std::cout << "Thread " << i << ": ";
		printResult(partialResult[i]);
		i++;
	}

	std::vector<char>result;
	for (int i = 0; i < numOfThreads; i++)
	{
		result.insert(result.end(), partialResult[i].begin(), partialResult[i].end());
	}
	
	std::cout << std::endl;
	std::cout << "Normalized result: " << std::endl;
	printResult(result);

	for (int i = 0; i < whitespaces.size(); i++)
	{
		result.insert(result.begin() + whitespaces[i], ' ');
	}

	std::cout << "Result: " << std::endl;
	printResult(result);
}
