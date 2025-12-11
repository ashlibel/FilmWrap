#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>
using namespace std;
// colors
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"
#define BRIGHT_BLACK   "\033[90m"
#define BRIGHT_RED     "\033[91m"
#define BRIGHT_GREEN   "\033[92m"
#define BRIGHT_YELLOW  "\033[93m"
#define BRIGHT_BLUE    "\033[94m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_CYAN    "\033[96m"
// effects
#define BLINK "\033[5m"
#define DIM            "\033[2m"

int extractYear(const string& date) {
    if (date.size() < 4) return 0;
    return stoi(date.substr(0, 4));
}

string extractYearMonth(const string& date) {
    if (date.size() < 7) return "";
    return date.substr(0, 7);
}

string monthLabel(const string& yearMonth) { // could change the csv file
    if (yearMonth.size() < 7) return yearMonth;
    string year = yearMonth.substr(0, 4);
    string month = yearMonth.substr(5, 2);
    string name;
    if (month == "01") name = "January";
    else if (month == "02") name = "February";
    else if (month == "03") name = "March";
    else if (month == "04") name = "April";
    else if (month == "05") name = "May";
    else if (month == "06") name = "June";
    else if (month == "07") name = "July";
    else if (month == "08") name = "August";
    else if (month == "09") name = "September";
    else if (month == "10") name = "October";
    else if (month == "11") name = "November";
    else if (month == "12") name = "December";
    else name = "Unknown";
    return name + " " + year;
}

//methods for effects
string rainbowText(const string& text) {
    const string colors[] = {
        "\033[91m", "\033[93m", "\033[92m",
        "\033[96m", "\033[94m", "\033[95m"
    };
    string result;
    int i = 0;
    for (char c : text) {
        if (c == ' ') { result += c; continue; }
        result += colors[i % 6] + string(1, c);
        i++;
    }
    result += RESET;
    return result;
}

void typeWriter(const string& msg, int delayMs = 20) {
    for (char c : msg) {
        cout << c << flush;
        this_thread::sleep_for(chrono::milliseconds(delayMs));
    }
    cout << "\n";
}

void fadeIn(const string& msg) {
    cout << DIM << msg << RESET << "\r" << flush;
    this_thread::sleep_for(chrono::milliseconds(250));
    cout << msg << "\n";
}

string gradientBar(int count) {
    const string colors[] = {
        "\033[91m", "\033[93m", "\033[92m",
        "\033[96m", "\033[94m", "\033[95m"
    };
    string bar;
    for (int i = 0; i < count; ++i) {
        bar += colors[i % 6] + "#" + RESET;
    }
    return bar;
}

void loadingScreen(const string& userName) { //to make it feel more authehtic
    cout << CYAN << "\nCrunching your movie stats, " << userName << RESET;
    cout.flush();
    for (int i = 0; i < 3; ++i) {
        cout << ".";
        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(400));
    }
    cout << "\n\n";
}

string starString(double rating) {
    int fullStars = static_cast<int>(floor(rating));  
    bool hasHalf = (rating - fullStars >= 0.5 - 1e-9);
    int used = fullStars + (hasHalf ? 1 : 0);
    const int totalSlots = 5;

    string s;
    for (int i = 0; i < fullStars; ++i) s += "✭"; // full star
    if (hasHalf) s += "☆";// suppose to represent half stars
    for (int i = used; i < totalSlots; ++i) s += "☆";

    stringstream ss;
    ss << fixed << setprecision(1) << rating;
    s += " (" + ss.str() + "/5)";
    return s;
}

struct Movie {
    string title;
    string genre;
    string director;
    double rating;// .5-5 stars similar to 5 star rating on letterboxd
    int runtime;// mins
    string date;// yyyy/mm/dd

    Movie() : rating(0.0), runtime(0) {}
    Movie(const string& t, const string& g, const string& d, double r, int rt, const string& dt)
        : title(t), genre(g), director(d), rating(r), runtime(rt), date(dt) {}
};

class MovieBST {
    struct Node {
        Movie data;
        Node* left;
        Node* right;
        Node(const Movie& m) : data(m), left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;

    Node* insert(Node* node, const Movie& m) {
        if (!node) return new Node(m);
        if (m.rating > node->data.rating) {
            node->left = insert(node->left, m);
        } else {
            node->right = insert(node->right, m);
        }
        return node;
    }

    void inorderTopN(Node* node, int& n, vector<Movie>& out) const {
        if (!node || n <= 0) return;
        inorderTopN(node->left, n, out);
        if (n > 0) {
            out.push_back(node->data);
            --n;
        }
        inorderTopN(node->right, n, out);
    }

    void clear(Node* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

public:
    ~MovieBST() { clear(root); }

    void insert(const Movie& m) { root = insert(root, m); }

    vector<Movie> topN(int n) const {
        vector<Movie> res;
        inorderTopN(root, n, res);
        return res;
    }
};

class MovieManager {
private:
    vector<Movie> allMovies;                   
    stack<Movie> recentViewed;                
    queue<string> watchNext;                   
    unordered_map<string, int> genreCount; 
    unordered_map<string, int> directorCount;  
    unordered_map<string, vector<string>> directorGraph; 
    MovieBST ratingTree;                    

public:

    void addMovie(const Movie& m) {
        allMovies.push_back(m);
        recentViewed.push(m);
        genreCount[m.genre]++;
        directorCount[m.director]++;
        ratingTree.insert(m);
    }

    void buildDirectorGraphByDate() {
        if (allMovies.size() < 2) return;
        vector<Movie> sorted = allMovies;
        sort(sorted.begin(), sorted.end(),
             [](const Movie& a, const Movie& b) { return a.date < b.date; });
        for (size_t i = 1; i < sorted.size(); ++i) {
            string d1 = sorted[i - 1].director;
            string d2 = sorted[i].director;
            if (d1 != d2) {
                directorGraph[d1].push_back(d2);
                directorGraph[d2].push_back(d1);
            }
        }
    }

    bool importCSV(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Could not open file: " << filename << "\n";
            return false;
        }

        string line;
        if (!getline(file, line)) return false;
        while (getline(file, line)) {
            if (line.empty()) continue;

            stringstream ss(line);
            string title, yearStr, director, runtimeStr, genre, date, ratingStr;

            getline(ss, title, ',');
            getline(ss, yearStr, ',');
            getline(ss, director, ',');
            getline(ss, runtimeStr, ',');
            getline(ss, genre, ',');
            getline(ss, date, ',');
            getline(ss, ratingStr, ',');

            if (title.empty()) continue;

            int runtime = 0;
            double starRating = 0.0;
            try {
                runtime     = stoi(runtimeStr);
                starRating  = stod(ratingStr); 
            } catch (...) {
                continue;
            }
            if (starRating < 0.5) starRating = 0.5;
            if (starRating > 5.0) starRating = 5.0;

            Movie m(title, genre, director, starRating, runtime, date);
            addMovie(m);
        }

        buildDirectorGraphByDate();
        return true;
    }

    void showAllMovies() const {
        if (allMovies.empty()) {
            cout << RED << "No movies loaded yet.\n" << RESET;
            return;
        }

        cout << BOLD << "Movies in your history (" << allMovies.size() << "):" << RESET << "\n\n";

        for (const auto& m : allMovies) {
            cout 
                << BRIGHT_MAGENTA << m.title << RESET                       
                << " | " << BRIGHT_CYAN << m.genre << RESET            
                << " | " << BRIGHT_YELLOW << m.director << RESET           
                << " | " << BRIGHT_GREEN << starString(m.rating) << RESET   
                << " | " << BRIGHT_BLUE << m.runtime << " min" << RESET
                << " | " << DIM << m.date << RESET                       
                << "\n";
        }
    }

    void showWrappedSummary(const string& userName) const {
        if (allMovies.empty()) {
            cout << RED << "No movies loaded. Import a CSV first.\n" << RESET;
            return;
        }
        loadingScreen(userName);
  
        cout << MAGENTA << BOLD;
        cout << "—∘∙∘–—–∘∙∘——∘∙∘–—–∘∙∘——∘∙∘–—–∘∙∘——∘∙∘–—–∘∙∘—\n";
        cout << rainbowText("MOVIE WRAPPED 2025") << "\n";
        cout << MAGENTA << BOLD << "—∘∙∘–—–∘∙∘——∘∙∘–—–∘∙∘——∘∙∘–—–∘∙∘——∘∙∘–—–∘∙∘—\n";
        cout << RESET << "\n";

        fadeIn("Your 2025 movie journey, all in one place...");

        int total = 0;
        int total2025 = 0;
        double ratingSum2025 = 0.0;
        int runtimeSum2025 = 0;

        unordered_map<string, int> monthCount2025;   
        unordered_map<string, int> dayCount2025;    
        vector<int> ratingFreq(11, 0);         

        int longestRuntime = -1;
        Movie longestMovie;

        for (const auto& m : allMovies) {
            ++total;
            int year = extractYear(m.date);
            if (year == 2025) {
                ++total2025;
                ratingSum2025 += m.rating;
                runtimeSum2025 += m.runtime;

                int bucket = static_cast<int>(round(m.rating * 2.0));
                if (bucket >= 1 && bucket <= 10) {
                    ratingFreq[bucket]++;
                }

                string ym = extractYearMonth(m.date);
                if (!ym.empty()) monthCount2025[ym]++;
                dayCount2025[m.date]++;
                if (m.runtime > longestRuntime) {
                    longestRuntime = m.runtime;
                    longestMovie = m;
                }
            }
        }

        cout << CYAN;
        cout << "Total movies in history: " << total << "\n";
        cout << "Movies watched in 2025: " << total2025 << "\n";
        if (total2025 > 0) {
            double avgStar = ratingSum2025 * 1.0 / total2025;
            cout << "Average rating in 2025: "
                 << fixed << setprecision(2) << avgStar << " / 5★\n";
            cout << "Average runtime in 2025: "
                 << runtimeSum2025 * 1.0 / total2025 << " minutes\n";
        }
        cout << RESET << "\n";

        cout << BOLD << "Favorite genres (by count):\n" << RESET;
        if (genreCount.empty()) {
            cout << "  (no data)\n";
        } else {
            string favGenre;
            int maxGenreCount = 0;
            for (const auto& [g, c] : genreCount) {
                if (c > maxGenreCount) {
                    maxGenreCount = c;
                    favGenre = g;
    
                }
            }
            for (const auto& [g, c] : genreCount) {
                cout << "  " << g << " : "
                     << gradientBar(c) << " (" << c << ")\n";
            }

            if (!favGenre.empty()) {
                cout << "\nBased on your watch history, your core vibe is: "
                     << YELLOW << favGenre << RESET << ".\n";
                if (favGenre == "Horror")
                    cout << "You clearly like fear. Respect.\n";
                else if (favGenre == "Romance")
                    cout << "You’re a softie at heart.\n";
                else if (favGenre == "Sci-Fi")
                    cout << "Your imagination is literally in another galaxy.\n";
                else if (favGenre == "Drama")
                    cout << "You love emotional depth and storytelling.\n";
                else if (favGenre == "Comedy")
                    cout << "You chase laughs. Solid life choice.\n";
            }
        }

        string favDir;
        int maxDirCount = 0;
        for (const auto& [d, c] : directorCount) {
            if (c > maxDirCount) {
                maxDirCount = c;
                favDir = d;
            }
        }
        if (!favDir.empty()) {
            cout << "\nMost watched director: " << GREEN << favDir << RESET
                 << " (" << maxDirCount << " films)\n";
        }

        if (total2025 > 0 && longestRuntime > 0) {
            cout << "\nYour longest movie of 2025 was: " << BOLD << longestMovie.title << RESET
                 << " at " << longestRuntime << " minutes.\n";
        }

        if (!monthCount2025.empty()) { //most watched month 
            string bestYM;
            int bestYMCount = 0;
            for (const auto& [ym, c] : monthCount2025) {
                if (c > bestYMCount) {
                    bestYMCount = c;
                    bestYM = ym;
                }
            }
            cout << "\nYour most cinematic month of 2025: "
                 << MAGENTA << monthLabel(bestYM) << RESET
                 << " with " << bestYMCount << " movies.\n";
        }

        if (!dayCount2025.empty()) {
            string bestDay;
            int bestDayCount = 0;
            for (const auto& [day, c] : dayCount2025) {
                if (c > bestDayCount) {
                    bestDayCount = c;
                    bestDay = day;
                }
            }
            cout << "Your biggest binge day: " << bestDay
                 << " with " << bestDayCount << " movie(s).\n";
        }

        int bestBucket = -1;
        int bestRatingFreq = 0;
        for (int i = 1; i <= 10; ++i) {
            if (ratingFreq[i] > bestRatingFreq) {
                bestRatingFreq = ratingFreq[i];
                bestBucket = i;
            }
        }
        if (bestBucket != -1 && bestRatingFreq > 0) {
            double starsVal = bestBucket / 2.0;
            cout << "\nYour most common rating in 2025 was "
                 << fixed << setprecision(1) << starsVal << "★ / 5"
                 << " (" << bestRatingFreq << " time(s)).\n";
        }

        cout << BOLD << "\nTop rated movies overall:\n" << RESET;
        vector<Movie> top = ratingTree.topN(3);
        if (top.empty()) {
            cout << "  (no data)\n";
        } else {
            int rank = 1;
            for (const auto& m : top) {
                cout << "  " << rank++ << ". "
                     << "✨ " << BRIGHT_MAGENTA << m.title << RESET
                     << " ✨  " << BRIGHT_GREEN << starString(m.rating) << RESET
                     << "\n";
            }
        }
        cout << "\n" << CYAN
             << "Thanks for watching movies with me this year, "
             << userName << "! See you in your next wrap. 🎬\n"
             << RESET << "\n";
    }

    void showMovies2025() const {
        cout << BOLD << "Movies watched in 2025:\n" << RESET;
        bool any = false;
        for (const auto& m : allMovies) {
            if (extractYear(m.date) == 2025) {
                any = true;
                cout << "- " << m.title << " (" << m.date
                     << ", " << m.genre
                     << ", " << starString(m.rating) << ")\n";
            }
        }
        if (!any) cout << "  (none)\n";
    }

    void showRatingDistribution() const {
        cout << BOLD << "Rating distribution (all movies):\n" << RESET;

        if (allMovies.empty()) {
            cout << "  (no data)\n";
            return;
        }

        vector<int> freq(11, 0); 
        int total = 0;
        for (const auto& m : allMovies) {
            int bucket = static_cast<int>(round(m.rating * 2.0));
            if (bucket >= 1 && bucket <= 10) {
                freq[bucket]++;
                total++;
            }
        }

        if (total == 0) {
            cout << "  (no valid ratings)\n";
            return;
        }

        for (int b = 10; b >= 1; --b) {
            if (freq[b] == 0) continue;
            double starsVal = b / 2.0;
            cout << "  " << fixed << setprecision(1) << starsVal << "★ / 5: "
                 << gradientBar(freq[b])
                 << "  (" << freq[b] << ")\n";
        }

        cout << "\nYou’ve rated " << total << " movies in total.\n";
    } 
    void sortByTitle() { //bonus option: the sorting of binary search
        sort(allMovies.begin(), allMovies.end(),
             [](const Movie& a, const Movie& b) {
                 return a.title < b.title;
             });
    }

    int binarySearchTitle(const string& title) const {
        int lo = 0;
        int hi = static_cast<int>(allMovies.size()) - 1;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            if (allMovies[mid].title == title) return mid;
            if (allMovies[mid].title < title) lo = mid + 1;
            else hi = mid - 1;
        }
        return -1;
    }

    void searchMovieByTitle() {
        if (allMovies.empty()) {
            cout << RED << "No movies to search.\n" << RESET;
            return;
        }
        sortByTitle();
        cout << "Enter title to search (or 0 to return): ";
        string title;
        getline(cin, title);
        if (title == "0") {
            cout << "Returning to main menu.\n";
            return;
        }
        int idx = binarySearchTitle(title);
        if (idx == -1) {
            cout << RED << "Movie not found in your history.\n" << RESET;
        } else {
            const auto& m = allMovies[idx];
            cout << GREEN << "Found: " << RESET
                 << m.title << " | " << m.genre << " | " << m.director
                 << " | " << starString(m.rating)
                 << " | " << m.runtime << " min | " << m.date << "\n";
            recentViewed.push(m);
        }
    }

    void showDirectorGraph() const {
        cout << BOLD << "Your director journey (graph edges):\n" << RESET;
        if (directorGraph.empty()) {
            cout << "  (not enough data to build graph)\n";
            return;
        }
        int shown = 0;
        for (const auto& [d, neighbors] : directorGraph) {
            cout << CYAN << "  " << d << RESET;
            if (!neighbors.empty()) cout << "\n     " << YELLOW << "↓" << RESET << "\n     ";
            for (size_t i = 0; i < neighbors.size(); ++i) {
                cout << neighbors[i];
                if (i + 1 < neighbors.size()) cout << " " << MAGENTA << "--" << RESET << " ";
            }
            cout << "\n\n";
            if (++shown >= 5) break;
        }
    }

    void showRecentViewed() {
        cout << BOLD << "Recently viewed movies (stack top first):\n" << RESET;
        if (recentViewed.empty()) {
            cout << "  (none yet)\n";
            return;
        }
        stack<Movie> tmp = recentViewed;
        int count = 0;
        while (!tmp.empty() && count < 5) {
            cout << "  " << tmp.top().title
                 << " (" << tmp.top().date << ")\n";
            tmp.pop();
            ++count;
        }
    }

    void addToWatchNextManual() {
        cout << "Enter a movie title to add to your watch-next queue\n";
        cout << "(or type 0 to go back to the main menu): ";
        string title;
        getline(cin, title);

        if (title == "0") {
            cout << "Cancelled. Returning to main menu.\n";
            return;
        }

        if (title.empty()) {
            cout << "Empty title. Nothing added.\n";
            return;
        }

        watchNext.push(title);
        cout << GREEN << "Added \"" << title << "\" to your watch-next queue.\n" << RESET;
    }

    void watchNextMovie() {
        if (watchNext.empty()) {
            cout << RED << "Your watch-next queue is empty.\n" << RESET;
            return;
        }
        string title = watchNext.front();
        watchNext.pop();
        cout << YELLOW << "Now watching (simulated): " << title << RESET << "\n";
    }

    void showWatchNextQueue() const {
        if (watchNext.empty()) {
            cout << "Your watch-next queue is empty.\n";
            return;
        }
        cout << BOLD << "Your watch-next queue:\n" << RESET;
        queue<string> temp = watchNext;
        int pos = 1;
        while (!temp.empty()) {
            cout << "  " << pos++ << ". " << temp.front() << "\n";
            temp.pop();
        }
    }

    void removeFromWatchNextByTitle() {
        if (watchNext.empty()) {
            cout << RED << "Your watch-next queue is empty.\n" << RESET;
            return;
        }
        cout << "Enter the exact title to remove (or 0 to cancel): ";
        string title;
        getline(cin, title);
        if (title == "0") {
            cout << "Cancelled. Returning to main menu.\n";
            return;
        }

        queue<string> temp;
        bool removed = false;
        while (!watchNext.empty()) {
            if (!removed && watchNext.front() == title) {
                removed = true;   // skip this one
            } else {
                temp.push(watchNext.front());
            }
            watchNext.pop();
        }
        watchNext = temp;

        if (removed) {
            cout << GREEN << "Removed \"" << title << "\" from your watch-next queue.\n" << RESET;
        } else {
            cout << RED << "Title not found in your watch-next queue.\n" << RESET;
        }
    }
};

void printMenu(const string& userName) {
    cout << "Hey " << YELLOW << userName << RESET << ", choose what you want to see:\n" << RESET;
    cout << CYAN << "1."  << RESET << BRIGHT_CYAN << " Show all movies in your history\n" << RESET;
    cout << CYAN << "2."  << RESET << BRIGHT_BLUE << " Show your 2025 Movie Wrapped summary\n" << RESET;
    cout << CYAN << "3."  << RESET << BRIGHT_CYAN << " Show rating distribution (star histogram)\n" << RESET;
    cout << CYAN << "4."  << RESET << BRIGHT_BLUE  << " Search for a movie by title (binary search)\n" << RESET;
    cout << CYAN << "5."  << RESET << BRIGHT_CYAN << " Show your director journey graph\n" << RESET;
    cout << CYAN << "6."  << RESET << BRIGHT_BLUE << " Show recently viewed movies (stack)\n" << RESET;
    cout << CYAN << "7."  << RESET << BRIGHT_CYAN << " Add a movie title to watch-next queue\n" << RESET;
    cout << CYAN << "8."  << RESET << BRIGHT_BLUE << " Show watch-next queue\n" << RESET;
    cout << CYAN << "9."  << RESET << BRIGHT_CYAN << " Watch next movie from queue\n" << RESET;
    cout << CYAN << "10." << RESET << BRIGHT_BLUE << " Remove a movie from watch-next queue\n" << RESET;
    cout << CYAN << "11." << RESET << " " << BLINK << RED << "Exit" << RESET << "\n";
    cout << "(" << MAGENTA << "Hint" << RESET << ": type " << YELLOW << "42" << RESET << " for a secret mode)\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    typeWriter(string(BOLD) + "Welcome to Movie Wrapped 2025!" + RESET, 25);
    cout << "Enter your name: ";
    string userName;
    getline(cin, userName);
    if (userName.empty()) userName = "Friend";

    MovieManager manager;

    cout << "Enter CSV filename with your movie history (e.g., movies.csv): ";
    string filename;
    getline(cin, filename);

    if (!manager.importCSV(filename)) {
        cout << RED << "Failed to load CSV. Make sure the file is in the same folder and has the correct format.\n" << RESET;
    } else {
        cout << GREEN << "Imported movies from " << filename << ".\n" << RESET;
    }

    while (true) {
        cout << "\n" << BLUE << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" << RESET;
        cout << BOLD << userName << "'s Movie Wrapped Menu\n" << RESET;
        cout << BLUE << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" << RESET;
        printMenu(userName);

        cout << YELLOW << "Enter option number: " << RESET;
        int choice;
        if (!(cin >> choice)) {
            cout << RED << "Invalid input (not a number). Exiting.\n" << RESET;
            break;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear leftover newline

        cout << GREEN << "You selected option " << choice << "." << RESET << "\n";

        if (choice == 42) {
            cout << YELLOW << "\nSecret mode unlocked! Deep-dive wrap incoming...\n" << RESET;
            manager.showWrappedSummary(userName);
            continue;
        }

        switch (choice) {
            case 1:
                manager.showAllMovies();
                break;
            case 2:
                manager.showWrappedSummary(userName);
                break;
            case 3:
                manager.showRatingDistribution();
                break;
            case 4:
                manager.searchMovieByTitle();
                break;
            case 5:
                manager.showDirectorGraph();
                break;
            case 6:
                manager.showRecentViewed();
                break;
            case 7:
                manager.addToWatchNextManual();
                break;
            case 8:
                manager.showWatchNextQueue();
                break;
            case 9:
                manager.watchNextMovie();
                break;
            case 10:
                manager.removeFromWatchNextByTitle();
                break;
            case 11:
                cout << "Goodbye, " << userName << "!\n" << "Until next year 😌 \n";
                return 0;
            default:
                cout << RED << "Please enter a number between 1 and 11 (or 42 😉).\n" << RESET;
                break;
        }
    }

    return 0;
}