#include "wildcard.h"

bool contains_wildcard_character(const string &s) {
    for (int i = 0; i < (int) s.length(); i++) {
        if (s[i] == '*' || s[i] == '?') return true;
    } return false;
}

bool contains_tilde(const string& s) {
    return s[0] == '~';
}

string expand_tilde(const string& s) {
    string rest_s = s.substr(1, s.length() - 1);
    string res = getpwuid(getuid())->pw_dir + rest_s;
    cout << res << "\n";
    return res;
}

bool wildcard_matches(const string& s, const string& wildcard) {
    int n = s.length();
    int m = wildcard.length();

    if (m == 0) return (n == 0);
 
    bool dp[n + 1][m + 1];
    memset(dp, false, sizeof(dp));
 
    dp[0][0] = true;

    for (int j = 1; j <= m; j++) {
        if (wildcard[j - 1] == '*') dp[0][j] = dp[0][j - 1];
    }
        
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            if (wildcard[j - 1] == '*') {
                dp[i][j] = dp[i][j - 1] || dp[i - 1][j];
            } else if (wildcard[j - 1] == '?' || s[i - 1] == wildcard[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = false;
            }
        }
    }
 
    return dp[n][m];
}

void find_directories(vector<string>& wildcard_parts, int wc_it, string current_dir, vector<string>& directories) {
    if (wc_it == (int) wildcard_parts.size()) {
        directories.push_back(current_dir);
        return;
    }

    if (!contains_wildcard_character(wildcard_parts[wc_it])) {
        current_dir += wildcard_parts[wc_it];
        if (wc_it < (int) (wildcard_parts.size() - 1)) {
            current_dir += "/";
        } find_directories(wildcard_parts, wc_it + 1, current_dir, directories); 
        
    } else {
        for (const auto& entry : fs::directory_iterator(current_dir)) {
            const auto filename = entry.path().filename().string();
            if (wildcard_matches(filename, wildcard_parts[wc_it])) {
                current_dir += filename;
                if (wc_it < (int) (wildcard_parts.size() - 1)) {
                    current_dir += "/";
                } find_directories(wildcard_parts, wc_it + 1, current_dir, directories);
                
                if (wc_it < (int) (wildcard_parts.size() - 1)) {
                    current_dir = current_dir.substr(0, current_dir.length() - 1);
                } current_dir = current_dir.substr(0, current_dir.length() - filename.length());
            }
        }
    }
}

vector<string> get_directories(string wildcard) {

    vector<string> directories;

    if (wildcard[0] != '~' && wildcard[0] != '/') {
        char cwd[1024];
        wildcard = getcwd(cwd, sizeof(cwd)) + ("/" + wildcard);
    } else if (wildcard[0] == '~') {
        string rest_wildcard = wildcard.substr(1, wildcard.length() - 1);
        wildcard = getpwuid(getuid())->pw_dir + rest_wildcard;
    }

    wildcard = wildcard.substr(1, wildcard.length() - 1) + "/";
    stringstream wildcard_stream(wildcard);
    vector<string> wildcard_parts;
    string wildcard_part;
    while (true) {
        getline(wildcard_stream, wildcard_part, '/');
        if (wildcard_part != "") {
            wildcard_parts.push_back(wildcard_part);
        } else {
            break;
        }
    }
    
    string current_dir = "/";
    find_directories(wildcard_parts, 0, current_dir, directories);

    reverse(directories.begin(), directories.end());

    return directories;
}

vector<string> expand_wildcards(const vector<string>& args) {
    vector<string> new_args;
    for (const string& arg : args) {
        if (!contains_wildcard_character(arg)) {
            new_args.push_back(contains_tilde(arg)? expand_tilde(arg) : arg);
        } else {
            vector<string> expanded_arg = get_directories(arg);
            new_args.insert(new_args.end(), expanded_arg.begin(), expanded_arg.end());
        }
    } return new_args;
}
