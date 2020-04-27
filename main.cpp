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
void genKey(string name);
struct Wallet{
	string name, path, password, seed, xprv, xpub, address, pub;
	void createWallet(string, string, string);
};
struct MultisigWallet{
	string name, path, seed, password, xprv, xpub;
	string address, redeemScript;
	Wallet cosigner1, cosigner2, cosigner3;
	void createMultisigAddress(string, string, string, Wallet, Wallet, Wallet);
};
//void createSteg(string walletName, string multisigName, string recipientName);
struct Steg{
	string name, recipientName;
	Wallet wallet;
	MultisigWallet multisigName;
	//string Steame = multisigName+" "+walletName;
	void createSteg(MultisigWallet, Wallet, string);
	void decryptSteg();

};



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

void Wallet::createWallet(string inputName, string inputPath, string inputPassword){
	name = inputName; path = inputPath; password = inputPassword;
	cout << "CURRENT WALLET: " << name << endl;
	// Generate the wallet
	string createWallet = "electrum create --wallet "+path+"/"+name+" --password "+password;
	int flag = system(createWallet.c_str());
	// Add wallet's seed to object
	string seedFile = name+".seed";														
	string seedRequest = "electrum getseed --wallet "+path+"/"+name+" --password "+password+" > "+seedFile;
	flag = system(seedRequest.c_str());
	seed = getStringFromFile(seedFile);													
	// Add wallet's master private key (xprv) to object
	string xprvFile = name+".xprv";
	string xprvRequest = "electrum getmasterprivate --wallet "+path+"/"+name+" --password "+password+" > "+xprvFile;
	flag = system(xprvRequest.c_str());
	xprv = getStringFromFile(xprvFile);
	// Add wallet's master public key (mpk, xpub) to object
	string xpubFile = name+".xpub";
	string xpubRequest = "electrum getmpk --wallet "+path+"/"+name+" > "+xpubFile;
	flag = system(xpubRequest.c_str());
	xpub = getStringFromFile(xpubFile);
	// Add unused wallet address
	string addressFile = name+".address";
	string addressRequest = "electrum getunusedaddress --wallet "+path+"/"+name+" > "+addressFile;
	flag = system(addressRequest.c_str());
	address = getStringFromFile(addressFile);
	// Add public key for wallet address
	string pubFile = name+".pub";
	string pubRequest = "electrum getpubkeys "+address+" --wallet "+path+"/"+name+" > "+pubFile;
	flag = system(pubRequest.c_str());
	reformatPubFile(pubFile);
	pub = getStringFromFile(pubFile);
	return;
}

void MultisigWallet::createMultisigAddress(string inputName, string inputPath, string inputPassword, Wallet cosigner1, Wallet cosigner2, Wallet cosigner3){
	name = inputName; path = inputPath; password = inputPassword;
	string multisigRequest = "electrum createmultisig 2 '[\""+cosigner1.pub+"\", \""+cosigner2.pub+"\", \""+cosigner3.pub+"\"]' --wallet "+path+"/"+name+" > "+"tempFile.txt";
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
	string createAddressFile = "echo "+address+" > "+name+".address";
	flag = system(createAddressFile.c_str());
	// Clean up multisig redeemScript and output to file
	redeemScript.erase(redeemScript.end()-1, redeemScript.end());
	redeemScript.erase(redeemScript.begin());
	string createRsFile = "echo "+redeemScript+" > "+name+".rs";
	flag = system(createRsFile.c_str());
	// 
	string multisigBalance = "echo balance > "+name+".balance";
	flag = system(multisigBalance.c_str());
	return;
}

void Steg::createSteg(MultisigWallet multisig, Wallet wallet, string recipientName){
	name = multisig.name+"_cosigner"+wallet.name[wallet.name.length()-1];
	// Tar.gz the wallet, wallet peripheries, multisig RS into one folder
	string tarStegFiles = "tar -czf "+name+".tar.gz "+multisig.name+".address "+multisig.name+".rs "+multisig.name+".balance "+wallet.name+" "+wallet.name+".address "+wallet.name+".pub "+wallet.name+".seed "+wallet.name+".xprv "+wallet.name+".xpub";
	int flag = system(tarStegFiles.c_str());
	// Encrypt the tar.gz folder with AES
	string genAESpassword = "od -t x8 "+multisig.name+".address > "+name+".password";
	flag = system(genAESpassword.c_str());
	reformatAESpasswordFile(name+".password");
	string AESpassword = getStringFromFile(name+".password");
	string encryptAES = "openssl aes-256-cbc -a -in "+name+".tar.gz -out "+name+".tar.gz.enc -pbkdf2 -pass pass:"+AESpassword;
	flag = system(encryptAES.c_str());
	// Encrypt the password using recipient's RSA pub key, put everything into one final file before encoding in image
	string encryptRSA = "openssl rsautl -encrypt -in "+name+".password -out "+name+".password.enc -pubin -inkey "+recipientName+"key.pub";
	flag = system(encryptRSA.c_str());
	string tarRemainingFiles = "tar -czf "+name+" "+name+".tar.gz.enc "+name+".password.enc";
	flag = system(tarRemainingFiles.c_str());/**/
	return;
}

void Steg::decryptSteg(){
	string multisigName, walletName;
	// Decompress the steg
	string decompressSteg = "tar -xzf "+name;
	int flag = system(decompressSteg.c_str());
	// Decrypt the RSA-encrypted password
	string decryptRSA = "openssl rsautl -decrypt -inkey "+SELF_NAME+".pem -in "+name+".password.enc -out "+name+".password";
	flag = system(decryptRSA.c_str());
	// Decrypt the AES-encrypted tar.gz folder
	string AESpassword = getStringFromFile(name+".password");
	string decryptAES = "openssl aes-256-cbc -d -a -in "+name+".tar.gz.enc -out "+name+".tar.gz -pbkdf2 -pass pass:"+AESpassword;
	flag = system(decryptAES.c_str());
	// Decompress the folder with the wallet and multisig information
	string decompressFolder = "tar -xzf "+name+".tar.gz";
	flag = system (decompressFolder.c_str());
	return;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int main(){
    cout << "Hello World!" << endl;
	// STEP 0 - ESTABLISH THE CONTEXT
    // Alice generates her wallet, adds $1 mBTC
    Wallet w0;
	w0.createWallet("wallet0",CURRENT_PATH,"000");
    // Bob generates his 3 wallets
    Wallet w1, w2, w3;
	w1.createWallet("wallet1",CURRENT_PATH,"123");
	w2.createWallet("wallet2",CURRENT_PATH,"456");
	w3.createWallet("wallet3",CURRENT_PATH,"789");
    // Bob generates a multisig address from the 3 wallets
	MultisigWallet ms1;
	ms1.createMultisigAddress("multisig1",CURRENT_PATH,"012",w1,w2,w3);
    // Alice sends $1 mBTC to Bob's multisig address



	// STEP 1 - BOB GIVES MULTISIG WALLET TO EVE VIA IMAGES
    // Eve generates her RSA key pair, informs Bob of public key
   	genKey("EVE");
    // Bob encrypts each wallet+multisig into an individual steg (stegonograph)
	Steg steg1, steg2, steg3;
	steg1.createSteg(ms1,w1,"TEST");
	steg2.createSteg(ms1,w2,"TEST");
	steg3.createSteg(ms1,w3,"TEST");
    // Each steg is encoded into an image
    // Bob sends all three encoded images to Eve



	// STEP 2 - EVE RECEIVES THE IMAGES, OPENS WALLET
 	// Eve decodes each image to extract its embedded steg
	// Eve decrypt each steg and collects all the wallets info inside
	Steg steg1, steg2, steg3;
	cout << "Enter name of first steg to decrypt: ";
	cin >> steg1.name;
	steg1.decryptSteg();
	cout << "Enter name of second steg to decrypt: ";
	cin >> steg2.name;
	steg2.decryptSteg();
	cout << "Enter name of third steg to decrypt: ";
	cin >> steg3.name;
	steg3.decryptSteg();
	// Eve checks the amount available in the multisig address
	// Eve requests an amount from the multisig address, approves the transaction from 2 of the 3 wallets
    
    return 0;
}
