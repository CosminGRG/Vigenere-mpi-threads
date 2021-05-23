// Decryption using Vigenere Cipher
// Input: Message in UPPER CASE, Key in lowercase

#include "mpi.h"
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
#define NOMINMAX
#include <Windows.h>

#define ROOT_RANK 0
#define DECRYPT 1
#define ENCRYPT 2

int numberOfCharsInArray(char* array) {
	return strlen(array);
}

bool isSpace(unsigned char c) {
	return (c == ' ');
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

std::string normalizeInput(std::string str)
{
	std::vector<int> spaceIndex;
	str.erase(std::remove_if(str.begin(), str.end(), isSpace), str.end());

	return str;
}

std::string decryptMessage(char* str, char* key, int strLength)
{
	std::string decrypted;

	int j = 0;
	for (int i = 0; i < strLength; i++)
	{
		if (str[i] != ' ' && str[i] != '\t' && !ispunct(str[i]))
		{
			char c = (str[i] - key[j] + 26) % 26;
			c += 'A';

			decrypted.push_back(c);
			j++;
		}
		else
		{
			decrypted.push_back(str[i]);
		}
	}
	return decrypted;
}

std::string encryptMessage(char* str, char* key, int strLength, int keyLength)
{
	std::string encrypted;
	encrypted.push_back('p');
	int j = 0;
	for (int i = 0; i < strLength; i++)
	{
		int c = (unsigned char)str[i];
		if (isupper(c))
		{
			c = ((c - 'A') + (key[j++ % keyLength] - 'a')) % 26 + 'A';
		}
		else if (islower(c))
		{
			c = ((c - 'a') + (key[j++ % keyLength] - 'a')) % 26 + 'a';
		}

		encrypted.push_back(c);
	}

	return encrypted;
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

int main()
{

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);

	std::string str;
	std::string key;

	std::string result;

	int rank;
	int numOfTasks;

	int option = 0;

	int strLength = 0;
	int keyLength = 0;

	std::vector<int> whitespaces;

	int error = MPI_Init(NULL, NULL);
	if (error != MPI_SUCCESS) {
		std::cout << "Error starting MPI program. Terminating.\n";
		MPI_Abort(MPI_COMM_WORLD, error);
	}

	char strChar[255];
	char keyChar[255];

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numOfTasks);

	MPI_Status status;

	if (rank == ROOT_RANK)
	{
		std::cout << "Enter the message (255 chars max): " << std::endl;
		std::getline(std::cin, str);

		if (str.size() > 255)
		{
			std::cout << "Input is longer than 255 chars";
			MPI_Abort(MPI_COMM_WORLD, error);
		}

		std::cout << "Enter the cypher key: " << std::endl;
		std::cin >> key;

		std::cout << "Choose operation - > 1 to decrypt / 2 to encrypt:" << std::endl;
		std::cin >> option;

		SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
		std::cout << "String is: " << str << std::endl;
		std::cout << "Key is: " << key << std::endl;

		keyLength = key.size();

		std::cout << "Key length before rounding is " << keyLength << std::endl;

		// Make the length of the key the same as the string by repeating it.
		key = generateKey(str, key);

		std::cout << "Key after rounding: " << key << std::endl;

		strLength = str.size();
		keyLength = key.size();
		
		std::cout << "String length is " << strLength << std::endl;
		std::cout << "Key length after rounding is " << keyLength << std::endl;
		
		for (int i = 0; i < strLength; i++)
		{
			if (str[i] == ' ')
			{
				whitespaces.push_back(i);
			}
		}
		
		str = normalizeInput(str);
		std::cout << "Normalized input: " << str << std::endl;
		std::cout << std::endl;

		strcpy_s(strChar, str.c_str());
		strcpy_s(keyChar, key.c_str());

		/* Debug
		*  std::cout << decryptMessage(strChar, keyChar, strLength) << std::endl;
		*/
		SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED);
	}
	MPI_Bcast(&strChar, 255, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&keyChar, 255, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&strLength, 1, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&keyLength, 1, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&option, 1, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);

	// Elements remaining after division among processes
	int remainder = strLength % numOfTasks;
	// Number of elements per process
	int quotient = strLength / numOfTasks;

	// Calculate computation interval for each rank
	int start = rank * quotient + std::min(rank, remainder);
	int stop = (rank + 1) * quotient + std::min(rank + 1, remainder);
	
	// Local string to store local rank result
	std::string localStr;

	/* Debug 
	*  std::cout << "Rank " << rank << " received: " << strChar << " and " << keyChar << std::endl;
	*  std::cout << "Rank " << rank << " will decrypt from " << start << " to " << stop << std::endl;
	*/

	if (option == ENCRYPT)
	{
		int j = start;
		for (int i = start; (i < stop) && (strChar[i] != '\0'); i++)
		{
			char c;
			char s = strChar[i];
			char k = keyChar[i];

			c = encryptChar(s, k);

			localStr.push_back(c);
			j++;
		}
	}
	else if (option == DECRYPT)
	{
		int j = start;
		for (int i = start; (i < stop) && (strChar[i] != '\0'); i++)
		{
			char c;
			char s = strChar[i];
			char k = keyChar[j];

			c = decryptChar(s, k);

			localStr.push_back(c);
			j++;
		}
	}
	else
	{
		std::cout << "Invalid option" << std::endl;
		MPI_Abort(MPI_COMM_WORLD, error);
	}

	std::cout << "Rank " << rank << ": -> localString: " << localStr << std::endl;

	char charResult[255];
	if (rank == ROOT_RANK)
	{
		result.append(localStr);
		
		for (int i = 1; i < numOfTasks; i++)
		{
			MPI_Recv(&charResult, 255, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);
			result.append(charResult);
		}

		std::cout << "Normalized result: " << result << std::endl;

		for (int i = 0; i < whitespaces.size(); i++)
		{
			result.insert(result.begin() + whitespaces[i], ' ');
		}
		
		std::cout << "Result: " << result << std::endl;
	}
	else
	{
		strcpy_s(charResult, localStr.c_str());
		charResult[numberOfCharsInArray(charResult) + 1] = '\0';
		MPI_Send(&charResult, 255, MPI_CHAR, ROOT_RANK, 0, MPI_COMM_WORLD);
	}
	

	MPI_Finalize();
}