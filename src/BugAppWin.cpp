#include "Bug.h"

#include <algorithm>
#include <time.h>
#include <vector>
using namespace std;

int main_app(int ac, char** av)
{
	po::variables_map vm;
	po::options_description desc("Allowed Options");
	// declare arguments (boost)
	desc.add_options()
		("bug_file(s)", po::value<vector<string>>()->multitoken(), "Provide relative path to \"bug.txt\" file(s)")
		("landscape_file", po::value<std::string>()->required(), "Provide relative path to \"landscape.txt\" file");
	try 
	{
		unsigned int NumOfLines = 0;
#ifdef CHECK_TIME
		clock_t begin, end;
		double time_spent;
		begin = clock();
#endif
		po::store(po::parse_command_line(ac, av, desc), vm);
		vector<string> vBugFilesPaths;
		if (!vm["bug_file(s)"].empty())
			vBugFilesPaths = vm["bug_file(s)"].as<vector<string> >();
		else
			throw exception("No Bug(s) file paths provided!");

		std::cout << "Inputed arguments are:" << endl;
		auto iBugFilesPaths = vBugFilesPaths.begin();
		while (iBugFilesPaths != vBugFilesPaths.end())
		{
			if (iBugFilesPaths == vBugFilesPaths.begin())
				std::cout << "bug_file(s):" << endl << *iBugFilesPaths << endl;
			else
				std::cout << *iBugFilesPaths << endl;

			iBugFilesPaths++;
		}
		std::cout << " landscape_file:" << endl << vm["landscape_file"].as<std::string>() << std::endl;

		po::notify(vm);						// for reporting exception

		iBugFilesPaths = vBugFilesPaths.begin();
#ifndef MULTI_THREAD
		while (iBugFilesPaths != vBugFilesPaths.end())
		{
			string sFileName = (*iBugFilesPaths).substr((*iBugFilesPaths).rfind(DIR_SEPARATOR) + 1,
				(*iBugFilesPaths).rfind('.') - (*iBugFilesPaths).rfind(DIR_SEPARATOR) - 1);
			CBug<string, CONTAINER<string>::iterator, CONTAINER> bubica(sFileName);

			if (bubica.OnInit(iBugFilesPaths, vm["landscape_file"].as<std::string>()) == CBug<string, CONTAINER<string>::iterator, CONTAINER>::EFileOpenErrors::ALL_SUCCESSFULL)
			{
				cout << "Input files are correctly opened and loaded in memory." << '\n';
			}
			else
				cout << "Input files are not correctly opened!!!" << '\n';

			cout << "Number of lines:" << (NumOfLines = CBug<string, CONTAINER<string>::iterator, CONTAINER>::GetNumOfLines()) << '\n';
			bubica.NumOfBugs();
			cout << "Number of found bugs:" << (NumOfLines = bubica.GetNumOfBugs()) << '\n';
			iBugFilesPaths++;
		}
#else
		while (iBugFilesPaths != vBugFilesPaths.end())
		{
			CBug<string, CONTAINER<string>::iterator, CONTAINER>::EFileOpenErrors eFileOpen;
			if ((eFileOpen = CBug<string, CONTAINER<string>::iterator, CONTAINER>::OnInit(iBugFilesPaths, vm["landscape_file"].as<std::string>())) != CBug<string, CONTAINER<string>::iterator, CONTAINER>::EFileOpenErrors::ALL_SUCCESSFULL)
			{
				cout << CBug<string, CONTAINER<string>::iterator, CONTAINER>::mapFileErrors.at(eFileOpen);
				if (eFileOpen == CBug<string, CONTAINER<string>::iterator, CONTAINER>::EFileOpenErrors::LANDSCAPE_FAIL)
				{
					cout << vm["landscape_file"].as<std::string>() << endl;
					return -1;
				}
				else if (eFileOpen == CBug<string, CONTAINER<string>::iterator, CONTAINER>::EFileOpenErrors::BUG_FAIL)
					cout << *iBugFilesPaths << endl;
				else
					cout << endl;
				iBugFilesPaths++;
				continue;
			}

			cout << "Number of lines:" << (NumOfLines = CBug<string, CONTAINER<string>::iterator, CONTAINER>::GetNumOfLines()) << '\n';
	
			string sFileName = (*iBugFilesPaths).substr((*iBugFilesPaths).rfind(DIR_SEPARATOR)+1,
				(*iBugFilesPaths).rfind('.')-(*iBugFilesPaths).rfind(DIR_SEPARATOR)-1);
			vector<std::thread> vThreads;
			vector<unique_ptr<CBug<string, CONTAINER<string>::iterator, CONTAINER>>> vupBugs;
			for (unsigned int i = 0; i < NumOfLines / LINES_PER_THREAD + 1; i++)
			{
				vupBugs.push_back((unique_ptr<CBug<string, CONTAINER<string>::iterator, CONTAINER>>)(DBG_NEW CBug<string, CONTAINER<string>::iterator, CONTAINER>(sFileName)));
				vThreads.push_back(std::thread(std::ref(*(vupBugs.back().get())), i*LINES_PER_THREAD));				// must use std::ref() to avoid object copying to created thread
			}

			auto iupBugs = vupBugs.begin();
			for (std::thread& rThread : vThreads)
			{
				if (rThread.joinable())
					rThread.join();
				cout << "Number of Bugs (tid=" << ((*iupBugs).get())->GetThreadId() << ")=" << ((*iupBugs).get())->GetNumOfBugs() << "\n";
				iupBugs++;
			}
			vThreads.erase(vThreads.begin(), vThreads.end());
			vupBugs.erase(vupBugs.begin(), iupBugs);
			cout << "Total number of Bugs: " << CBug<string, CONTAINER<string>::iterator, CONTAINER>::GetTotNumOfBugs() << "\n";

			iBugFilesPaths++;
		}
#endif
#ifdef CHECK_TIME
		end = clock();
		time_spent = (double)(end - begin) / (CLOCKS_PER_SEC / 1000);
		cout << "Time of program execution is:" << time_spent << "ms\n";
#endif
		return 0;
	}
	catch (std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		std::cout << desc << std::endl;
		return 1;
	}
}

int main(int ac, char** av)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);		// check memory automatically after the program terminates (also static objects)

	main_app(ac, av);

    return 0;
}
