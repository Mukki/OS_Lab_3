/* Mission acceptée, Agent 00Pat. */

#include <iostream>
#include <fstream>

using namespace std;

#define PatOctet uint8_t /* Besoin d'octet? Utilisez des PatOctet! */

class HDD
{
private:
	HDD()
	{
		ofstream file("HD.DH", ios::binary | ios::out | ios::trunc);

        file.seekp(16383);
        file.write("\u0000", 1); /* FAT rempli de 0 */
        
        
        for (int pointer = 6; pointer < 256; pointer++) /* Remplis les 6 premiers blocs de table correctement */
        {
            file.seekp(pointer);
            file.write("\u0001", 1);
        }

		{ /* Bloc pour le test */
			file.seekp(6);
			file.write("\u0007", 1);
			file.write("\u0008", 1);
			file.write("\u0000", 1);

			file.seekp(256);
			file.write("a.txt\u0006", 6);
			
			file.seekp(384);
			file.write("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 64);
			file.write("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 64);
			file.write("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 32);

			file.seekp(9);
			file.write("\u0010", 1);
			file.write("\u0000", 1);

			file.seekp(262);
			file.write("b.txt\u0009", 6);

			file.seekp(576);
			file.write("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", 64);
			file.write("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", 32);
		}

		file.close();
	}

	void readBlock(int noBlock, char* tempRead)
	{
		ifstream file("HD.DH", ios::binary | ios::in);

		file.seekg(noBlock * 64);
		file.read(tempRead, 64);

		file.close();
	}

	void writeBlock(int noBlock, char* tempWrite)
	{
		fstream file("HD.DH", ios::binary | ios::out | ios::in);

		file.seekp(noBlock * 64);
		file.write(tempWrite, 64);

		file.close();
	}
	
	friend class PatOSStandardLibrary; /* Le HDD est private, la classe doit donc être ami.
			La raison : est-il normal de laisser la possibilié de lire et écrire les blocs publics?
			Poser la question, c'est y répondre! */
};

class PatOSStandardLibrary
{
private:
	HDD leDur; /* Le dur */

	char doesFileExist(char* filename)
	{
		char file[64];
        
        for (int j = 4; j < 6; j++) /* On lit dans les blocs de noms */
        {
            leDur.readBlock(j, file);
		
            for (int i = 0; i < 64; i += 6)
            {
                if (file[i] == filename[0])
                {
                    return file[i + 5];
                }
            }
        }
        
        return 0; /* Si n'existe pas, retourne 0 */
	}

	char nextBlock(int block)
	{
		char tempBlock[64];

        int i = 0;
        
        while (block > 63) /* Pour trouver dans quel bloc de table le prochain est */
        {
            block -= 64;
            i++;
        }
        
		leDur.readBlock(i, tempBlock);

		return tempBlock[block];
	}

    char getNewBlock(char oldblock)
    {
        char tempBlock[64];
        
        for (int j = 0; j < 4; j++) /* On cherche dans les blocs de le table */
        {
            leDur.readBlock(j, tempBlock);
            
            for (int i = 0; i < 64; i++) /* Parmis toutes les cases du bloc */
            {
                if (tempBlock[i] == 1)
                {
					leDur.readBlock(j, tempBlock);

					tempBlock[i] = 0b00000000;

					leDur.writeBlock(j, tempBlock);

					if (oldblock != 0)
					{
						char temp = oldblock;
						int k = 0;
						while (oldblock > 63)
						{
							k++;
							temp -= 64;
						}

						leDur.readBlock(k, tempBlock);

						tempBlock[oldblock] = (char)64 * j + i;

						leDur.writeBlock(k, tempBlock);
					}
                    return 64 * j + i;
                }
            }
        }
        
        cout << "Plus de place libre: Vider la corbeille!" << endl;
        
        return 0;
    }

	char writeFileToHDD(char* tempWrite, int position, char place)
	{
		char tempWrite1[64];
		char tempWrite2[64];

		leDur.readBlock(place, tempWrite1);

		return 0;
	}

	char writeFilenameToHDD(char* filename, char place)
	{
		char tempBlock[64];

		for (int j = 4; j < 6; j++)
		{
			leDur.readBlock(4, tempBlock);

			for (int i = 0; i < 60; i += 6) /* On a juste 10 entrées max (64 / 6 = 10) */
			{
				if (tempBlock[i] == 0)
				{
					for (int k = 0; k < 5; k++)
					{
						tempBlock[i + k] = filename[k];
					}
					tempBlock[i + 5] = place;

					leDur.writeBlock(j, tempBlock);
					
					return 1;
				}
			}
		}

		cout << "Plus de place libre: Vider la corbeille!" << endl;

		return 0;
	}

public:
	void read(char* filename, int position, int noChar, char* tempRead)
	{

	}

	void write(char* filename, int position, int noChar, char* tempWrite)
	{
		char tempBlock[64];

		char place = doesFileExist(filename);

		if (place)
		{
			while (position > 63) /* Permet de trouver le bloc pour débuter l'écriture */
			{
				place = nextBlock(place);
				position -= 64;
			}
		}
		else
		{
			place = getNewBlock(0);
			writeFilenameToHDD(filename, place); /* Écrit en mémoire le nom du fichier */
		}

		while (noChar > 0) 
		{
			writeFileToHDD(tempBlock, position, place); /* Pour écrire en bloc de 64 octets */
			noChar -= 64;
			char oldblock = place;
			place = nextBlock(place);
			if (place == 0)
				place = getNewBlock(oldblock);
		}
	}

	void deleteEOF(char* filename, int position)
	{

	}
};

int main()
{
	PatOSStandardLibrary PatOSHDD;

	PatOSHDD.write("c.txt", 0, 16, "cccccccccccccccc");

	return 0;
}