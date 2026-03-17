#include <iostream>
#include <fstream>
#include <random>
#include <set>

using namespace std;

void generate_dataset(int id, int num_courses, int num_lecturers, int num_groups, int num_rooms) {
    string prefix = "dataset_" + to_string(id);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> lec_dist(0, num_lecturers - 1);
    uniform_int_distribution<> group_count_dist(1, 3);
    uniform_int_distribution<> group_dist(0, num_groups - 1);
    uniform_int_distribution<> size_dist(20, 50);
    uniform_int_distribution<> cap_dist(30, 100);
    uniform_int_distribution<> undes_count(0, 5);
    uniform_int_distribution<> time_dist(0, 44); // 45 timeslots
    
    // courses.csv: id,lecturer,duration,groups
    ofstream courses(prefix + "_courses.csv");
    courses << "id,lecturer,duration,groups\n";
    for (int i = 0; i < num_courses; i++) {
        int lecturer = lec_dist(gen);
        int duration = 1;
        int num_grps = group_count_dist(gen);
        set<int> grps;
        while ((int)grps.size() < num_grps) {
            grps.insert(group_dist(gen));
        }
        courses << i << "," << lecturer << "," << duration << ",\"";
        bool first = true;
        for (int g : grps) {
            if (!first) courses << ";";
            courses << g;
            first = false;
        }
        courses << "\"\n";
    }
    courses.close();
    
    // rooms.csv: id,capacity
    ofstream rooms(prefix + "_rooms.csv");
    rooms << "id,capacity\n";
    for (int i = 0; i < num_rooms; i++) {
        rooms << i << "," << cap_dist(gen) << "\n";
    }
    rooms.close();
    
    // groups.csv: id,size
    ofstream groups(prefix + "_groups.csv");
    groups << "id,size\n";
    for (int i = 0; i < num_groups; i++) {
        groups << i << "," << size_dist(gen) << "\n";
    }
    groups.close();
    
    // lecturer_undesirable.csv: lecturer,timeslots
    ofstream lec_undes(prefix + "_lecturer_undesirable.csv");
    lec_undes << "lecturer,timeslots\n";
    for (int i = 0; i < num_lecturers; i++) {
        int count = undes_count(gen);
        set<int> times;
        while ((int)times.size() < count) {
            times.insert(time_dist(gen));
        }
        lec_undes << i << ",\"";
        bool first = true;
        for (int t : times) {
            if (!first) lec_undes << ";";
            lec_undes << t;
            first = false;
        }
        lec_undes << "\"\n";
    }
    lec_undes.close();
    
    // group_undesirable.csv: group,timeslots
    ofstream grp_undes(prefix + "_group_undesirable.csv");
    grp_undes << "group,timeslots\n";
    for (int i = 0; i < num_groups; i++) {
        int count = undes_count(gen);
        set<int> times;
        while ((int)times.size() < count) {
            times.insert(time_dist(gen));
        }
        grp_undes << i << ",\"";
        bool first = true;
        for (int t : times) {
            if (!first) grp_undes << ";";
            grp_undes << t;
            first = false;
        }
        grp_undes << "\"\n";
    }
    grp_undes.close();
    
    cout << "Generated dataset " << id << ": " << num_courses << " courses, " 
         << num_lecturers << " lecturers, " << num_groups << " groups, " 
         << num_rooms << " rooms" << endl;
}

int main() {
    cout << "Generating 5 datasets with exponentially increasing sizes..." << endl;
    
    // Base parameters
    int base_courses = 25;
    int base_lecturers = 5;
    int base_groups = 8;
    int base_rooms = 5;
    
    for (int i = 1; i <= 5; i++) {
        int multiplier = 1 << (i - 1); // 1, 2, 4, 8, 16
        generate_dataset(i, 
                        base_courses * multiplier,
                        base_lecturers * multiplier,
                        base_groups * multiplier,
                        base_rooms * multiplier);
    }
    
    cout << "\nAll datasets generated successfully!" << endl;
    cout << "Files created: dataset_X_*.csv where X is 1-5" << endl;
    
    return 0;
}
