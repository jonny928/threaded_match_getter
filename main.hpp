#include <iostream>
#include <string>
#include <list>
#include <curl/curl.h>
#include <thread>

using namespace std;

// Threaded site-fetching and interpretation.
template <typename T>
void create_threads(list <thread> * threads, void (*func)(T*), list <T> * arguments);
void join_threads(list <thread> * threads);
void threaded_read(string * buffer);

// This class and its usage is copied from John Hobbs (see credit).
//
// "There's no need to invent the wheel anew."
//                    - Every programmer ever
// 
// Credit: http://www.velvetcache.org/2008/10/24/better-libcurl-from-c
class curl_object {
public:
	curl_object(string url) {
		curl = curl_easy_init();
		if(!curl)
			throw string ("Curl did not initialize!");

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curl_object::curl_writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_buffer);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_perform(curl);
	};

	static int curl_writer(char* data, size_t size, size_t nmemb, string *buffer) {
		int result = 0;
		if (buffer != NULL) {
			buffer->append(data, size * nmemb);
			result = size * nmemb;
		}
		return result;
	}

	string get_data() {
		return curl_buffer;
	}

protected:
	CURL * curl;
	string curl_buffer;
};


/**********************************************************\
|* Hard-coded functions to extract data from  gosu-gamers *|
\**********************************************************/

// Find the start of a tag starting from start_index.
int find_start_of(const string& tag, const string& data, const int& start_index) {
  return data.find("<" + tag + ">", start_index);
}

// Find the end of a tag starting from start_index.
int find_end_of(const string& tag, const string& data, const int& start_index) {
  return data.find("</" + tag + ">", start_index);
}

// Find the next match-link on this page starting from start_index.
string find_match_link(const string & data, const int& start_index) {
  int link_start = data.find("<a href=\"", start_index) + 9;
  int link_length = data.find("\"", link_start) - link_start;
  return data.substr(link_start, link_length);
}

// Return the total amount of pages there are.
int get_total_pages(const string& data, const int& start_index) {

  // Find the "Showing x to y of z Matches"-part of the page.
  int view_start = data.find_first_not_of("\n ", data.find("<div class=\"viewing\">", start_index) + 22);
  int view_length = find_end_of("div", data, view_start) - view_start;
  string view = data.substr(view_start, view_length);
  view = view.substr(0, view.find_last_not_of(" ") + 1);

  // Read how many matches are left.
  int matches_start = view.find(" of ") + 4;
  int matches_length = view.find(" Matches") - matches_start;
  string matches = view.substr(matches_start, matches_length);
  int imatches = stoi(matches);

  // There are 15 matches per page.
  return imatches/15 + (imatches % 15 == 0 ? -1 : 0);
}

// Trim the name of a team to remove any unwanted suffixes.
string trim_opponent(const string& opp) {
	int dota_suffix_start = opp.find(" Dota 2");
	if (dota_suffix_start != string::npos) {
		return opp.substr(0, dota_suffix_start);
	}
	dota_suffix_start = opp.find(".Dota 2");
	if (dota_suffix_start != string::npos) {
		return opp.substr(0, dota_suffix_start);
	}
	dota_suffix_start = opp.find(" Dota2");
	if (dota_suffix_start != string::npos) {
		return opp.substr(0, dota_suffix_start);
	}
	dota_suffix_start = opp.find(".Dota2");
	if (dota_suffix_start != string::npos) {
		return opp.substr(0, dota_suffix_start);
	}
  return opp;
}

// Get the name of team 1 or 2 in the match on this page.
string get_opponent(const string& opp, const string& data) {
	int opp_start = data.find("<div class=\"opponent opponent" + opp + "\">") + 32;
	opp_start = data.find(">", data.find("<a href=\"", opp_start)) + 1;
	int opp_length = find_end_of("a", data, opp_start) - opp_start;

  return trim_opponent(data.substr(opp_start, opp_length));
}

// Get the time and date for the match on this page.
string get_datetime(const string& data) {
	int time_start = data.find_first_not_of("\n ", data.find("<p class=\"datetime is-upcomming\">") + 33);
	int time_length = find_end_of("p", data, time_start) - time_start;
	string time = data.substr(time_start, time_length);
	return time.substr(0, time.find_last_not_of(" ") + 1);
}
