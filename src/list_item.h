#include<QListWidgetItem>
#include<string>

class list_item : public QListWidgetItem {
public:
    list_item(int id, std::string name) :
            QListWidgetItem(0),
            id_(id),
            name_(name)
    {
    }

    int id()
    {
        return id_;
    }

    std::string name()
    {
        return name_;
    }

private:
    int id_;
    std::string name_;
};
