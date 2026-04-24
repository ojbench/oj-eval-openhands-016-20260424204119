#include <bits/stdc++.h>
using namespace std;

// Persistent key->sorted unique list<int> store on disk
// Binary format:
// uint32_t key_count
// repeat key_count times:
//   uint16_t key_len
//   char key[key_len]
//   uint32_t val_count
//   int32_t vals[val_count]  // sorted ascending

static const char* DATA_FILE = "bpt_datastore.bin";

using Store = unordered_map<string, vector<int>>;

static bool load_store(Store &mp) {
    ifstream fin(DATA_FILE, ios::binary);
    if (!fin) return false;
    uint32_t key_cnt = 0;
    fin.read(reinterpret_cast<char*>(&key_cnt), sizeof(key_cnt));
    if (!fin) { fin.close(); return false; }
    mp.reserve(key_cnt ? key_cnt * 2 : 0);
    for (uint32_t i = 0; i < key_cnt; ++i) {
        uint16_t klen = 0; fin.read(reinterpret_cast<char*>(&klen), sizeof(klen));
        if (!fin) { fin.close(); return false; }
        string key(klen, '\0');
        if (klen) fin.read(&key[0], klen);
        if (!fin) { fin.close(); return false; }
        uint32_t vcnt = 0; fin.read(reinterpret_cast<char*>(&vcnt), sizeof(vcnt));
        if (!fin) { fin.close(); return false; }
        vector<int> vals; vals.resize(vcnt);
        if (vcnt) fin.read(reinterpret_cast<char*>(vals.data()), vcnt * sizeof(int));
        if (!fin) { fin.close(); return false; }
        mp.emplace(std::move(key), std::move(vals));
    }
    fin.close();
    return true;
}

static bool save_store(const Store &mp) {
    ofstream fout(DATA_FILE, ios::binary | ios::trunc);
    if (!fout) return false;
    uint32_t key_cnt = static_cast<uint32_t>(mp.size());
    fout.write(reinterpret_cast<const char*>(&key_cnt), sizeof(key_cnt));
    if (!fout) return false;
    for (const auto &p : mp) {
        const string &key = p.first;
        const vector<int> &vals = p.second;
        uint16_t klen = static_cast<uint16_t>(key.size());
        fout.write(reinterpret_cast<const char*>(&klen), sizeof(klen));
        if (klen) fout.write(key.data(), klen);
        uint32_t vcnt = static_cast<uint32_t>(vals.size());
        fout.write(reinterpret_cast<const char*>(&vcnt), sizeof(vcnt));
        if (vcnt) fout.write(reinterpret_cast<const char*>(vals.data()), vcnt * sizeof(int));
        if (!fout) return false;
    }
    fout.close();
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Store store;
    load_store(store); // ignore failure => start empty

    int n; if (!(cin >> n)) return 0;
    string cmd, idx;
    for (int i = 0; i < n; ++i) {
        cin >> cmd;
        if (cmd == "insert") {
            int v; cin >> idx >> v;
            auto &vec = store[idx];
            auto it = lower_bound(vec.begin(), vec.end(), v);
            if (it == vec.end() || *it != v) vec.insert(it, v);
        } else if (cmd == "delete") {
            int v; cin >> idx >> v;
            auto itKey = store.find(idx);
            if (itKey != store.end()) {
                auto &vec = itKey->second;
                auto it = lower_bound(vec.begin(), vec.end(), v);
                if (it != vec.end() && *it == v) {
                    vec.erase(it);
                    if (vec.empty()) store.erase(itKey);
                }
            }
        } else if (cmd == "find") {
            cin >> idx;
            auto itKey = store.find(idx);
            if (itKey == store.end() || itKey->second.empty()) {
                cout << "null\n";
            } else {
                const auto &vec = itKey->second;
                for (size_t j = 0; j < vec.size(); ++j) {
                    if (j) cout << ' ';
                    cout << vec[j];
                }
                cout << '\n';
            }
        } else {
            // unknown command; consume rest of line just in case
            string rest; getline(cin, rest);
        }
    }

    save_store(store);
    return 0;
}
