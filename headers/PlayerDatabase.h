#ifndef PLAYER_DATABASE_H
#define PLAYER_DATABASE_H

#include <string>
#include <vector>

class PlayerDatabase
{
  public:
    struct Entry
    {
        std::string name;
        long long totalBlocks;
    };

    explicit PlayerDatabase(std::string path);

    void addBlocks(const std::string &name, long long count);
    std::vector<Entry> getSorted() const;

  private:
    void load();
    void save() const;

    std::string path_;
    std::vector<Entry> entries_;
};

#endif
