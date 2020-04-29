// Created by Cristian Morales, crism@bu.edu
// EC544 Project: Sharable Crypto Wallet
using namespace std;
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdio.h>


// GLOBAL VARIABLES
string CURRENT_PATH = "~/Desktop/EC544/test1";

// FUNCTION & STRUCT DEFINITIONS
string getStringFromFile(string filename);
void reformatPubFile(string filename);
void reformatAESpasswordFile(string filename);
void createWallet(string walletName);
void createMultisig(string multisigName, vector<string> cosignersList, int multisigThreshold);
void createSteg(string multisigName, string cosignerName, string keyName);
void genKey(string keyName);
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

void reformatAESpasswordFile(string filename){
	ifstream file(filename);
	string word, byte8;
	while(file >> word){
		if (word.length()==16){
			byte8 = word;
			break;
		}
	}
	file.close();
	string command = "echo "+byte8+" > "+filename;
	int flag = system(command.c_str());
	return;
}

void genKey(string name){
	// Generate private RSA key
	string genPemKey = "openssl genpkey -algorithm RSA -out "+name+".pem";
	int flag1 = system(genPemKey.c_str());
	// Generate public RSA key
	string genPubKey = "openssl rsa -in "+name+".pem -pubout -out "+name+".pub";
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

void createMultisig(string multisigName, vector<string> cosignersList, int multisigThreshold){
	string pub;
	string multisigRequest = "electrum createmultisig "+to_string(multisigThreshold)+" '[\"";
	for (int i = 0; i < cosignersList.size(); i++){
		pub = getStringFromFile(cosignersList[i]+".pub");
		multisigRequest.append(pub);
		multisigRequest.append("\", \"");
	}
	multisigRequest.erase(multisigRequest.end()-3, multisigRequest.end());
	multisigRequest = multisigRequest+"]' --wallet "+CURRENT_PATH+"/"+multisigName+" > "+"tempFile.txt";
	int flag = system(multisigRequest.c_str());
	ifstream file("tempFile.txt");
	string word, lastWord, address, redeemScript;
	while(file >> word){
		if (lastWord[1] == 'a') address = word;
		if (lastWord[1] == 'r') redeemScript = word;
		lastWord = word;
	}
	file.close();
	flag = system("rm tempFile.txt");
	// Clean up multisig address and output to file
	address.erase(address.end()-2, address.end());
	address.erase(address.begin());
	string createAddressFile = "echo "+address+" > "+multisigName+".address";
	flag = system(createAddressFile.c_str());
	// Clean up multisig redeemScript and output to file
	redeemScript.erase(redeemScript.end()-1, redeemScript.end());
	redeemScript.erase(redeemScript.begin());
	string createRsFile = "echo "+redeemScript+" > "+multisigName+".rs";
	flag = system(createRsFile.c_str());
	// Create a file with the multisig wallet balance
	string getMultisigBalance = "electrum getbalance --wallet "+CURRENT_PATH+"/"+cosignersList[0]+" > "+multisigName+".balance";
	flag = system(getMultisigBalance.c_str());
	return;
}

void createSteg(string multisigName, string cosignerName, string keyName){
	string stegName = multisigName+"_"+cosignerName;
	// Tar.gz the wallet, wallet peripheries, multisig RS into one folder
	string tarStegFiles = "tar -czf "+stegName+".tar.gz "+multisigName+".address "+multisigName+".rs "+multisigName+".balance "+cosignerName+" "+cosignerName+".address "+cosignerName+".pub "+cosignerName+".seed "+cosignerName+".xprv "+cosignerName+".xpub";
	int flag = system(tarStegFiles.c_str());
	// Encrypt the tar.gz folder with AES
	string genAESpassword = "od -t x8 "+multisigName+".address > "+stegName+".password";
	flag = system(genAESpassword.c_str());
	reformatAESpasswordFile(stegName+".password");
	string AESpassword = getStringFromFile(stegName+".password");
	string encryptAES = "openssl aes-256-cbc -a -in "+stegName+".tar.gz -out "+stegName+".tar.gz.enc -pbkdf2 -pass pass:"+AESpassword;
	flag = system(encryptAES.c_str());
	// Encrypt the password using recipient's RSA pub key, put everything into one final file before encoding in image
	string encryptRSA = "openssl rsautl -encrypt -in "+stegName+".password -out "+stegName+".password.enc -pubin -inkey "+keyName+".pub";
	flag = system(encryptRSA.c_str());
	string tarRemainingFiles = "tar -czf "+stegName+".bin "+stegName+".tar.gz.enc "+stegName+".password.enc";
	flag = system(tarRemainingFiles.c_str());
	// Convert the steg to base64 for encoding into the image
	string encodeBase64 = "openssl base64 -in "+stegName+".bin -out "+stegName;
	flag = system(encodeBase64.c_str());
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


// IF ACTING IN SENDER ROLE
int main(){
    int flag, multisigThreshold;
    vector<string> cosignersList;
	string input, keyName, walletName, status, stegName, multisigName, transactionAmount, cosignerName;
	while(1){
		cout << "PROGRAM: Enter '1' to create an Electrum wallet" << endl;
		cout << "         Enter '2' to create a transaction between two Electrum wallets" << endl;
		cout << "         Enter '3' to create an Electrum multisig wallet" << endl;
		cout << "         Enter '4' to create an encrypted steg containing the multisig info" << endl;
		cout << "         Enter '5' to encode a steg into an image" << endl;
		cout << "         Type 'exit' to exit the program" << endl;
		cout << "USER: ";
		cin >> input;
		cout << endl;
		if (input == "1"){
			cout << "PROGRAM: Please enter your desired Electrum wallet name." << endl << "USER: ";
			cin >> walletName;
			createWallet(walletName);
			cout << "PROGRAM: The Electrum wallet '" << walletName << "' has been created." << endl << endl;
		}
		if (input == "2"){
			// SEE OPTION '5' IN receiverMain.cpp main()
		}
		if (input == "3"){
			cout << "PROGRAM: Please enter your desired Electrum multisig wallet name." << endl << "USER: ";
			cin >> multisigName;
			cout << "PROGRAM: Please enter the names of the multisig wallet cosigners, one at a time." << endl << "         (When all cosigners are entered, type 'finished'.)" << endl;
			while (1){
				cout << "USER: ";
				cin >> cosignerName;
				if (cosignerName == "finished")
					break;
				else
					cosignersList.push_back (cosignerName);
			}
			cout << "PROGRAM: Please enter the number of cosigner signatures needed to" << endl << "         unlock funds from this multisig wallet." << endl << "USER: ";
			cin >> multisigThreshold;
			createMultisig(multisigName, cosignersList, multisigThreshold);
			cout << "PROGRAM: The Electrum multisig wallet '" << multisigName << "' has been created." << endl << endl;
		}
		if (input == "4"){
			cout << "PROGRAM: Please enter the name of the multisig wallet to be put into the steg." << endl << "USER: ";
			cin >> multisigName;
			cout << "PROGRAM: Please enter the name of the cosigner wallet to be put into the steg." << endl << "USER: ";
			cin >> cosignerName;
			cout << "PROGRAM: Please enter the name of the RSA key being used to encrypt the steg." << endl << "USER: ";
			cin >> keyName;
			createSteg(multisigName, cosignerName, keyName);
			cout << "PROGRAM: The steg '" << multisigName << "_" << cosignerName << "' has been created and encrypted." << endl << endl;
		}
		if (input == "5"){
			flag = system("gnome-terminal --window");
			cout << "PROGRAM: In the newly-opened terminal window, type in 'npm start'." << endl << "         Then open your browser and enter 'localhost:2222' in the address bar." << endl << "         Use this page to upload a steg .txt file and encode it into a" << endl << "         randomly-selected image. Do this for as many stegs as needed." << endl;
			while (1){
				cout << "PROGRAM: When you are finished encoding all the desired stegs into images," << endl << "         close your browser, hit CTRL-C, and close the window." << endl << "         Then enter 'finished' below." << endl << "USER: ";
				cin >> status;
				if (status == "finished"){
					cout << endl;
					break;
				}
			}
		}
		if (input == "exit"){
			break;
		}
		else if ((input != "1") && (input != "2") && (input != "3") && (input != "4") && (input != "5") && (input != "exit")){
			cout << "PROGRAM: Entered response not recognized. Plase type 'exit' if you wish to exit the program." << endl << endl;;
		}
		cout << endl;
	}
    return 0;
}

// IF ACTING IN RECEIVER ROLE
/*
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
			//cout << "PROGRAM: In the newly-opened terminal window, type in 'npm start'." << endl << "         Then open your browser and enter 'localhost:3000' in the address bar." << endl << "         Use this page to upload an image encoded with a steg and download " << endl << "         the resulting steg .txt file into this current folder." << endl << "         Do this for as many stegs as you need." << endl;
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
					//"cat signed2.txn | electrum broadcast -"
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
*/
