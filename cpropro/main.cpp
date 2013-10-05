#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <curl/curl.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
using boost::property_tree::ptree;

std::string account="uwcstest";
std::string password="uwcs";

std::string data; //will hold the url's contents
std::ofstream image_file("temp_image.jpg");

template<class T> std::string toString(const T& t)
{
	std::ostringstream stream;
	stream << t;
	return stream.str();
}

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up)
{ //callback must have this declaration
	//buf is a pointer to the data that curl has for us
	//size*nmemb is the size of the buffer

	for (int c = 0; c<size*nmemb; c++)
	{
		data.push_back(buf[c]);
	}
	return size*nmemb; //tell curl how many bytes we handled
}

size_t write_file(char* buf, size_t size, size_t nmemb, void* up)
{ //callback must have this declaration
	//buf is a pointer to the data that curl has for us
	//size*nmemb is the size of the buffer

	for (int c = 0; c<size*nmemb; c++)
	{
		image_file << buf[c];
	}
	return size*nmemb; //tell curl how many bytes we handled
}


int main()
{
	CURL* curl; //our curl object

	curl_global_init(CURL_GLOBAL_ALL); //pretty obvious
	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, "http://www.reddit.com/r/nocontext/random.json");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_perform(curl);

	ptree pt;
	std::stringstream ss;
	ss << data;
	boost::property_tree::read_json(ss, pt);
	ptree posts = pt.begin()->second.get_child("data.children");
//	for(ptree::iterator it=posts.begin(); it!=posts.end(); ++it)
//	{
//		std::cout << it->second.get<std::string>("data.title") << std::endl;
//	}
	ptree::iterator it=posts.begin();
	std::string title = it->second.get<std::string>("data.title");
	std::cout << "title: " << title << std::endl;

	data = "";
	curl_easy_setopt(curl, CURLOPT_URL, "http://version1.api.memegenerator.net/Instances_Select_ByPopular");
	curl_easy_perform(curl);

	ss << data;
	boost::property_tree::read_json(ss, pt);
	posts = pt.get_child("result");
//	for(ptree::iterator it=posts.begin(); it!=posts.end(); ++it)
//	{
//		std::cout << it->second.get<std::string>("imageUrl") << std::endl;
//	}
	it=posts.begin();
	std::string url = it->second.get<std::string>("imageUrl");
	std::cout << "imageurl: " << url << std::endl;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_file);
	curl_easy_perform(curl);
	/* always cleanup */
	image_file.close();
	curl_easy_cleanup(curl);

	std::string title1 = title.substr(0,title.length()/2);
	std::replace(title1.begin(), title1.end(), '"', ' ');
	std::string title2 = title.substr(title.length()/2);
	std::replace(title2.begin(), title2.end(), '"', ' ');

	std::system(std::string("convert temp_image.jpg -size 400 -background rgba\\(0,0,0,0\\) -font impact.ttf -strokewidth 2 -pointsize " + toString<int>(1200/title1.size()) + " -stroke black -fill white -gravity north caption:\"" + title1 + "\" -compose over -composite -pointsize " + toString<int>(1200/title2.size()) + " -gravity south caption:\"" + title2 + "\" -compose over -composite output.jpg").c_str());

	curl_global_cleanup();
	return 0;
}
