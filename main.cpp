#include "main.hpp"

using namespace std;

int main() {

  // List for all match-links, and later match-info. Rename maybe?
	list<string> links = list<string>();

  // Initiation for curl.
	curl_global_init(CURL_GLOBAL_ALL);

  // Get all match-links for upcoming matches.
	int page = 1;
	int remaining_pages = -1;
	do {
    // Get the current page.
		curl_object current_page("http://www.gosugamers.net/dota2/gosubet?u-page=" + to_string(page++));
		string data = current_page.get_data();

    // Find the first row in the table of upcoming matches.
		int starting_index = data.find("<h2>Dota 2 Upcoming Matches</h2>") + 23;
		int row = find_start_of("tr", data, starting_index + 4);

    // Add all match-links on this page to the list.
		while (find_end_of("table", data, starting_index) > row) {
			links.push_back("http://www.gosugamers.net" + find_match_link(data, row));
			row = find_start_of("tr", data, row + 1);
		}

    // The amount of remaining pages has not yet been established
		if (remaining_pages == -1) {
      remaining_pages = get_total_pages(data, starting_index);
		}
	} while (remaining_pages-- > 0);

  // Create a thread for each match and make them fetch match-info.
	list <thread> threads;
	create_threads(&threads, &threaded_read, &links);
	join_threads(&threads);

  // The threads have replaced the match-links with match-info.
	for (list<string>::iterator it = links.begin(); it != links.end(); it++) {
		cout << *it << endl;
	}

  // Cleanup for curl.
	curl_global_cleanup();

	return 0;
}

// For each argument in arguments, spawn a thread applying func to it.
template <typename T>
void create_threads(list <thread> * threads, void (*func)(T*), list <T> * arguments) {
	for(typename list<T>::iterator it = arguments->begin(); it != arguments->end(); it++) {
		threads->push_back(thread(*func, &(*it)));
	}
}

// Wait for all threads to finish.
void join_threads(list <thread> * threads) {
  while (!threads->empty()) {
		threads->front().join();
		threads->pop_front();
	}
}

// Fetch the web-page pointed to by buffer, fetching and formatting
// the match-info on the page and entering it into the buffer.
void threaded_read(string * buffer) {
  // Fetch web-page.
	curl_object current_page(buffer->c_str());
	string data = current_page.get_data();

  // Get the two teams facing eachother.
  string opp1 = get_opponent("1", data);
	string opp2 = get_opponent("2", data);

  // Get the start datetime.
  string time = get_datetime(data);

  // Format the match-info and place it in the buffer instead of the link.
	*buffer = time + ": " + opp1 + " - " + opp2;
}
