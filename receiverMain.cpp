using namespace std;
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdio.h>

// GLOBAL VARIABLES
string CURRENT_PATH = "~/Desktop/EC544/test/PHASE0/EVE";

// FUNCTION DEFINITIONS
string getStringFromFile(string filename);
void reformatPubFile(string filename);
void genKey(string keyName);
void createWallet(string walletName);
void decryptSteg(string stegName, string keyName);



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// FUNCTION & STRUCT IMPLEMENTATIONS
string getStringFromFile(string filename){
	ifstream file(filename);
	char fromFile_char[255]; string fromFile_string;
	while(file){
		file.getline(fromFile_char, 255);
		string s(fromFile_char);
		fromFile_string.append(s);
	}
	file.close();
	return fromFile_string;
}

void reformatPubFile(string filename){
	ifstream file(filename);
	char c; string s;
	while(file.get(c)){
		if ((c != '[') && (c != ' ') && (c != '"') && (c != ']') && (c != '\n'))
			s += c;
	}
	file.close();
	string command = "echo "+s+" > "+filename;
	int flag = system(command.c_str());
	return;
}

void genKey(string keyName){
	// Generate private RSA key
	string genPemKey = "openssl genpkey -algorithm RSA -out "+keyName+".pem";
	int flag1 = system(genPemKey.c_str());
	// Generate public RSA key
	string genPubKey = "openssl rsa -in "+keyName+".pem -pubout -out "+keyName+".pub";
	int flag2 = system(genPubKey.c_str());
	return;
}

void createWallet(string walletName){
	int flag;
	string name = walletName;
	// Generate the wallet
	string createWallet = "electrum create --wallet "+CURRENT_PATH+"/"+walletName;
	flag = system(createWallet.c_str());
	// Create file for wallet's seed
	string seedRequest = "electrum getseed --wallet "+CURRENT_PATH+"/"+walletName+" > "+walletName+".seed";
	flag = system(seedRequest.c_str());
	// Create file for wallet's master private key (xprv)
	string xprvRequest = "electrum getmasterprivate --wallet "+CURRENT_PATH+"/"+walletName+" > "+walletName+".xprv";
	flag = system(xprvRequest.c_str());
	// Create file for wallet's master public key (mpk, xpub)
	string xpubRequest = "electrum getmpk --wallet "+CURRENT_PATH+"/"+walletName+" > "+walletName+".xpub";
	flag = system(xpubRequest.c_str());
	// Create file for wallet's address
	string addressRequest = "electrum getunusedaddress --wallet "+CURRENT_PATH+"/"+walletName+" > "+walletName+".address";
	flag = system(addressRequest.c_str());
	string address = getStringFromFile(walletName+".address");
	// Create file for wallet's public key for the available address
	string pubRequest = "electrum getpubkeys "+address+" --wallet "+CURRENT_PATH+"/"+walletName+" > "+walletName+".pub";
	flag = system(pubRequest.c_str());
	reformatPubFile(walletName+".pub");
	return;
}

void decryptSteg(string stegName, string keyName){
	string multisigName, walletName;
	// Decode the steg from base64 to bin
	string decodeBase64 = "openssl base64 -d -in "+stegName+" -out "+stegName+".bin";
	int flag = system(decodeBase64.c_str());
	// Decompress the steg
	string decompressSteg = "tar -xzf "+stegName+".bin";
	flag = system(decompressSteg.c_str());
	// Decrypt the RSA-encrypted password
	string decryptRSA = "openssl rsautl -decrypt -inkey "+keyName+".pem -in "+stegName+".password.enc -out "+stegName+".password";
	flag = system(decryptRSA.c_str());
	// Decrypt the AES-encrypted tar.gz folder
	string AESpassword = getStringFromFile(stegName+".password");
	string decryptAES = "openssl aes-256-cbc -d -a -in "+stegName+".tar.gz.enc -out "+stegName+".tar.gz -pbkdf2 -pass pass:"+AESpassword;
	flag = system(decryptAES.c_str());
	// Decompress the folder with the wallet and multisig information
	string decompressFolder = "tar -xzf "+stegName+".tar.gz";
	flag = system (decompressFolder.c_str());
	return;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int main(){
    int flag;
	string input, keyName, walletName, status, stegName, multisigName, transactionAmount, cosignerName, cosigner1Name, cosigner2Name, cosigner3Name;	
	while(1){
		cout << "PROGRAM: Enter '1' to generate an RSA key pair" << endl;
		cout << "         Enter '2' to create an Electrum wallet" << endl;
		cout << "         Enter '3' to extract a steg from an encoded image" << endl;
		cout << "         Enter '4' to decrypt a steg" << endl;
		cout << "         Enter '5' to request funds from a multisig wallet" << endl;
		cout << "         Type 'exit' to exit the program" << endl;
		cout << "USER: ";
		cin >> input;
		cout << endl;
		if (input == "1"){
			cout << "PROGRAM: Please enter your desired RSA key name." << endl << "USER: ";
			cin >> keyName;
			genKey(keyName);
			cout << "PROGRAM: RSA key pair '" << keyName << ".pem' & '" << keyName << ".pub' has been generated." << endl << endl;
		}
		if (input == "2"){
			cout << "PROGRAM: Please enter your desired Electrum wallet name." << endl << "USER: ";
			cin >> walletName;
			createWallet(walletName);
			cout << "PROGRAM: The Electrum wallet '" << walletName << "' has been created." << endl << endl;
		}
		if (input == "3"){
			flag = system("gnome-terminal --window");
			cout << "PROGRAM: In the newly-opened terminal window, type in 'npm start'." << endl << "         Then open your browser and enter 'localhost:3000' in the address bar." << endl << "         Use this page to upload an image encoded with a steg and download " << endl << "         the resulting steg .txt file into this current folder." << endl << "         Do this for as many stegs as you need." << endl;
			while (1){
				cout << "PROGRAM: When you are finished decoding all the desired stegs, close " << endl << "         your browser, hit CTRL-C, and close the window." << endl << "         Then enter 'finished' below." << endl << "USER: ";
				cin >> status;
				if (status == "finished"){
					cout << endl;
					break;
				}
			}
		}
		if (input == "4"){
			cout << "PROGRAM: Please enter the name of the steg you would like to decrypt." << endl << "USER: ";
			cin >> stegName;
			cout << "PROGRAM: Please enter the name of the RSA key used to encrypt the steg." << endl << "USER: ";
			cin >> keyName;
			decryptSteg(stegName, keyName);
			cout << "PROGRAM: The steg '" << stegName << "' has been decrypted." << endl << endl;
		}
		if (input == "5"){
			cout << "PROGRAM: Please enter the name of the multisig wallet you are requesting funds from." << endl << "USER: ";
			cin >> multisigName;
			cout << "PROGRAM: This is the amount of bitcoin in that multisig wallet. (In the 'confirmed' row.)" << endl;
			string getMultisigBalance = "cat "+multisigName+".balance";
			flag =  system(getMultisigBalance.c_str());
			cout << endl << "PROGRAM: How much are you requesting? (Enter '!' for the max amount.)" << endl << "USER: ";
			cin >> transactionAmount;
			cout << "PROGRAM: Please enter the name of the Electrum wallet these funds should be sent to." << endl << "USER: ";	
			cin >> walletName;
			cout << "PROGRAM: Please enter the name of the first multisig wallet cosigner." << endl << "USER: ";
			cin >> cosignerName;
			// "electrum payto walletName_ADDRESS ! --from_addr MULTISIG_ADDRESS - > signed1.txn"
			while (1){
				cout << "PROGRAM: Please enter the name of an additional multisig wallet cosigner. (If none additional needed, enter 'finished'.)" << endl << "USER: ";
				cin >> cosignerName;
				if (cosignerName == "finished"){
					//"cat signed2.txn | electrum broadcast -"*/
					break;
				}
				else{
					//"cat signed1.txn | electrum signtransaction - > signed2.txn" 					
				}
			}
			cout << "PROGRAM: FINISHED TRANSACTION" << endl << endl;
		}
		if (input == "exit"){
			break;
		}
		else if ((input != "1") && (input != "2") && (input != "3") && (input != "4") && (input != "5") &&(input != "exit")){
			cout << "PROGRAM: Entered response not recognized. Plase type 'exit' if you wish to exit the program." << endl << endl;;
		}
		cout << endl;
	}
    return 0;
}
