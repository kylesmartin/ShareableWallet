using namespace std;
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdio.h>

// GLOBAL VARIABLES
string CURRENT_PATH = "~/Desktop/EC544/test1/EVE";
string SELF_NAME = "TESTkey";


// FUNCTION & STRUCT DEFINITIONS
string getStringFromFile(string filename);
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

void genKey(string name){
	// Generate private RSA key
	string genPemKey = "openssl genpkey -algorithm RSA -out "+name+".pem";
	int flag1 = system(genPemKey.c_str());
	// Generate public RSA key
	string genPubKey = "openssl rsa -in "+name+".pem -pubout -out "+name+".pub";
	int flag2 = system(genPubKey.c_str());
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
	// STEP 0 - ESTABLISHING THE CONTEXT
    // Alice generates her wallet, adds $1 mBTC
    // Bob generates his 3 wallets
    // Bob generates a multisig address from the 3 wallets
    // Alice sends $1 mBTC to Bob's multisig address



	// STEP 1 - BOB GIVES MULTISIG WALLET TO EVE VIA IMAGES
    // Eve generates her RSA key pair, informs Bob of public key
	genKey(SELF_NAME);
    // Bob encrypts each wallet+multisig into an individual steg (stegonograph)
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
