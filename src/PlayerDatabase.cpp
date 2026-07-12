#include "PlayerDatabase.h"
#include <algorithm>
#include <fstream>

PlayerDatabase::PlayerDatabase(std::string path) : path_(std::move(path)) { load(); }

void PlayerDatabase::load()
{
    entries_.clear();
    std::ifstream in(path_);
    if (!in.is_open())
        return;
    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;
        auto sep = line.rfind('|');
        if (sep == std::string::npos)
            continue;
        std::string name = line.substr(0, sep);
        long long total = 0;
        try
        {
            total = std::stoll(line.substr(sep + 1));
        }
        catch (...)
        {
            continue;
        }
        entries_.push_back({name, total});
    }
}

void PlayerDatabase::save() const
{
    std::ofstream out(path_, std::ios::trunc);
    if (!out.is_open())
        return;
    for (const auto &e : entries_)
        out << e.name << '|' << e.totalBlocks << '\n';
}

void PlayerDatabase::addBlocks(const std::string &name, long long count)
{
    if (count <= 0 || name.empty())
        return;
    for (auto &e : entries_)
    {
        if (e.name == name)
        {
            e.totalBlocks += count;
            save();
            return;
        }
    }
    entries_.push_back({name, count});
    save();
}

std::vector<PlayerDatabase::Entry> PlayerDatabase::getSorted() const
{
    auto sorted = entries_;
    std::sort(sorted.begin(), sorted.end(),
              [](const Entry &a, const Entry &b) { return a.totalBlocks > b.totalBlocks; });
    return sorted;
}
