#include "../h/msclStringFuncs.h"
#include "../h/PhplusProgram.h"
#include "../h/ErrorHandler.h"

#include <iostream>

using std::cout;
using std::endl;
using std::vector;
using std::string;

vector <string> cmdLineArgs;

struct Flags
{
	string myPath;					// path to the ph+ exeutable that is now running
	vector<string> inFiles;			// all the input files
	bool debug=false;				// if to show debugging info
	bool help=false;				// if to show help message
	bool version=false;				// if to show version message
	bool runInterpreted=true;		// if to run the program in the interpreter
	string cppOutFile="";			// output file for transpiled C++ code, empty if flag not set
	string binOutFile="";			// binary executable output file, empty if flag not set
	bool runCompiled=false;			// if to run the program compiled
	bool flagError=false;			// if to quit immediately, this is set if there is an unrecognised flag
};

Flags getFlags(int argc, char ** argv);

int main(int argc, char ** argv)
{
	Flags flags=getFlags(argc, argv);
	
	if (flags.flagError)
	{
		cout << "coba '-h' untuk menampilkan bantuan" << endl;
		return 0;
	}
	
	if (flags.help)
	{
		cout << "PHPLUS Versi " << VERSION_X << "." << VERSION_Y << "." << VERSION_Z << endl;
		cout << "usage: Phplus [options] [source file] [options]" << endl;
		cout << "\noptions: " << endl;
		cout << "-v, -version      Menampilkan versi dari Phplus" << endl;
		cout << "-d, -debug        Menampilkan info debugging sebelum menjalankan program" << endl;
		cout << "-r, -run          Menjalankan program dengan interpreter" << endl;
		cout << "                    	aktif secara default jika tidak ada transpiling commands yang dijalankan" << endl;
		cout << "                    	pada dasarnya, apapun setelah -r sudah tersetujui" << endl;
		cout << "-cpp [file]       Transpile ke C++ dan menyimpan output file yang diberikan" << endl;
		cout << "-bin [file]       Transpile, compile dengan GCC dan menyimpan binary" << endl;
		cout << "-e, -execute      Transpile, compile dan execute file binary" << endl;
		cout << "                    beberapa kombinasi dari -cpp, -bin and -e dapat digunakan" << endl;
		cout << "                    seperti -r, apapun setelah -e akan disetujui" << endl;
		cout << "-h, -help         Menampilkan bantuan saat ini dan keluar" << endl;
		cout << endl;
		cout << endl;
		
		return 0;
	}
	
	if (flags.version)
	{
		cout << "Phplus version " << VERSION_X << "." << VERSION_Y << "." << VERSION_Z << endl;
		return 0;
	}
	
	PhplusProgram program;
	
	if (flags.inFiles.empty())
	{
		cout << "Tidak ada sumber file yang spesifik" << endl;
		cout << "coba '-h' untuk menampilkan bantuan" << endl;
		return 0;
	}
	else if (flags.inFiles.size()>1)
	{
		cout << "Sumber file multiple spesifik, Phplus tidak mendukungnya saat ini" << endl;
		cout << "coba '-h' untuk menampilkan bantuan" << endl;
		return 0;
	}
	
	program.resolveProgram(flags.inFiles[0], flags.debug);
	
	if (flags.runInterpreted)
	{
		if (error.getIfErrorLogged())
		{
			if (flags.debug)
				cout << endl << ">>>>>>    Eksekusi dibatalkan karena terdapat error    <<<<<<" << endl;
			else
				cout << "Mohon bersabar, ini ujian\nprogram tidak dijalankan karena terdapat error" << endl;
		}
		else
		{
			if (flags.debug)
				cout << endl << "Menjalankan program..." << endl << endl;
			
			program.execute();
		}
	}
	
	if (!flags.cppOutFile.empty() || !flags.binOutFile.empty() || flags.runCompiled)
	{
		string cppCode=program.getCpp();
		
		if (error.getIfErrorLogged())
		{
			if (flags.debug)
				cout << endl << ">>>>>>    transpiling gagal   <<<<<<" << endl;
			else
				cout << "transpiling gagal" << endl;
		}
		else
		{
			string cppFileName=flags.cppOutFile;
			
			if (cppFileName.empty())
				cppFileName="tmp_pp_transpiled.cpp";
			
			if (flags.debug)
				cout << endl << putStringInBox(cppCode, "C++ code", true, false, -1) << endl;
			
			writeFile(cppFileName, cppCode, flags.debug);
			
			if (!flags.binOutFile.empty() || flags.runCompiled)
			{
				string binFileName=flags.binOutFile;
				
				if (binFileName.empty())
					binFileName="tmp_pp_compiled";
				
				string cmd;
				cmd="g++ -std=c++11 '"+cppFileName+"' -o '"+binFileName+"'";
				
				if (flags.debug)
					cout << "menjalankan '"+cmd+"'" << endl;
				
				runCmd(cmd, true);
				
				if (flags.runCompiled)
				{
					if (flags.debug)
						cout << endl;
					
					cmd = "./"+binFileName + " --menjalankan-dari-phplus " + str::join(cmdLineArgs, " ", false);
					
					if (flags.debug)
						cout << "menjalankan '"+cmd+"'" << endl << endl;
					
					runCmd(cmd, true);
				}
				
				if (flags.binOutFile.empty())
					remove(binFileName.c_str());
			}
			
			if (flags.cppOutFile.empty())
				remove(cppFileName.c_str());
		}
	}
	
	if (flags.debug)
		cout << endl << "Semua selesai" << endl;
	
	return 0;
}

Flags getFlags(int argc, char ** argv)
{
	bool after = false;
	Flags flags;
	
	for (int i=1; i<argc; i++)
	{
		string arg(argv[i]);
		if (!after)
		{
			if (arg.size()>1 && arg[0]=='-')
			{
				string flag=arg.substr(1, string::npos);

				if (flag=="d" || flag=="debug")
				{
					flags.debug=true;
				}
				else if (flag=="v" || flag=="version")
				{
					flags.version=true;
				}
				else if (flag=="h" || flag=="help")
				{
					flags.help=true;
				}
				else if (flag=="r" || flag=="run")
				{
					flags.runCompiled=false;
					flags.runInterpreted=true;
					after = true;
				}
				else if (flag=="cpp")
				{
					if (i+1>=argc)
					{
						cout << "output file harus diikuti '-cpp' flag";
						flags.flagError=true;
					}

					flags.runInterpreted=false;

					flags.cppOutFile=string(argv[i+1]);

					i++;
				}
				else if (flag=="bin")
				{
					if (i+1>=argc)
					{
						cout << "output file harus diikuti '-bin' flag";
						flags.flagError=true;
					}

					flags.runInterpreted=false;

					flags.binOutFile=string(argv[i+1]);

					i++;
				}
				else if (flag=="e" || flag=="execute")
				{
					flags.runCompiled=true;
					flags.runInterpreted=false;
					after = true;
				}
				else
				{
					cout << "flag tidak diketahui '"+flag+"'" << endl;
					flags.flagError=true;
				}
			}
			else
			{
				flags.inFiles.push_back(arg);
				cmdLineArgs.push_back(arg);
			}
		}
		else
		{
			cmdLineArgs.push_back(arg);
		}
	}
	
	return flags;
}


