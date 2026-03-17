#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <algorithm>
#include <cmath>
#include <limits>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace std;

struct Course {
    int id;
    int lecturer;
    set<int> groups;
    int duration;
};

struct Room {
    int id;
    int capacity;
};

struct Assignment {
    int course;
    int room;
    int timeslot;
};

struct Solution {
    vector<Assignment> assignments;
    double cost;
    int hard_violations;
    int gap_penalty;
    int undesirable_penalty;
};

class UCTPInstance {
public:
    int num_courses, num_lecturers, num_groups, num_rooms, num_timeslots;
    int num_days = 5, periods_per_day = 9;
    
    vector<Course> courses;
    vector<Room> rooms;
    vector<int> group_sizes;
    vector<set<int>> lecturer_undesirable;
    vector<set<int>> group_undesirable;
    
    double alpha = 10000.0, beta = 10.0, gamma = 5.0;
    
    vector<int> parse_int_list(const string& str) {
        vector<int> result;
        if (str.empty()) return result;
        stringstream ss(str);
        string item;
        while (getline(ss, item, ';')) {
            if (!item.empty()) {
                result.push_back(stoi(item));
            }
        }
        return result;
    }
    
    void load_from_csv(const string& prefix) {
        num_timeslots = num_days * periods_per_day;
        
        // Load courses
        ifstream courses_file(prefix + "_courses.csv");
        string line;
        getline(courses_file, line); // Skip header
        while (getline(courses_file, line)) {
            Course c;
            stringstream ss(line);
            string field;
            
            getline(ss, field, ','); c.id = stoi(field);
            getline(ss, field, ','); c.lecturer = stoi(field);
            getline(ss, field, ','); c.duration = stoi(field);
            getline(ss, field, ',');
            // Remove quotes
            field.erase(remove(field.begin(), field.end(), '\"'), field.end());
            for (int g : parse_int_list(field)) {
                c.groups.insert(g);
            }
            courses.push_back(c);
        }
        num_courses = courses.size();
        courses_file.close();
        
        // Load rooms
        ifstream rooms_file(prefix + "_rooms.csv");
        getline(rooms_file, line); // Skip header
        while (getline(rooms_file, line)) {
            Room r;
            stringstream ss(line);
            string field;
            getline(ss, field, ','); r.id = stoi(field);
            getline(ss, field, ','); r.capacity = stoi(field);
            rooms.push_back(r);
        }
        num_rooms = rooms.size();
        rooms_file.close();
        
        // Load groups
        ifstream groups_file(prefix + "_groups.csv");
        getline(groups_file, line); // Skip header
        while (getline(groups_file, line)) {
            stringstream ss(line);
            string field;
            getline(ss, field, ','); // id
            getline(ss, field, ','); // size
            group_sizes.push_back(stoi(field));
        }
        num_groups = group_sizes.size();
        groups_file.close();
        
        // Load lecturer undesirable
        lecturer_undesirable.resize(100); // Max lecturers
        ifstream lec_undes_file(prefix + "_lecturer_undesirable.csv");
        getline(lec_undes_file, line); // Skip header
        while (getline(lec_undes_file, line)) {
            stringstream ss(line);
            string field;
            getline(ss, field, ',');
            int lec = stoi(field);
            getline(ss, field, ',');
            field.erase(remove(field.begin(), field.end(), '\"'), field.end());
            for (int t : parse_int_list(field)) {
                lecturer_undesirable[lec].insert(t);
            }
        }
        lec_undes_file.close();
        
        // Count actual lecturers
        set<int> lec_set;
        for (const auto& c : courses) lec_set.insert(c.lecturer);
        num_lecturers = lec_set.size();
        
        // Load group undesirable
        group_undesirable.resize(num_groups);
        ifstream grp_undes_file(prefix + "_group_undesirable.csv");
        getline(grp_undes_file, line); // Skip header
        while (getline(grp_undes_file, line)) {
            stringstream ss(line);
            string field;
            getline(ss, field, ',');
            int grp = stoi(field);
            getline(ss, field, ',');
            field.erase(remove(field.begin(), field.end(), '\"'), field.end());
            for (int t : parse_int_list(field)) {
                group_undesirable[grp].insert(t);
            }
        }
        grp_undes_file.close();
        
        cout << "Loaded: " << num_courses << " courses, " << num_lecturers 
             << " lecturers, " << num_groups << " groups, " << num_rooms << " rooms" << endl;
    }
    
    bool check_overlap(int t1, int t2, int duration) const {
        return abs(t1 - t2) < duration;
    }
    
    int count_hard_violations(const Solution& sol) const {
        int violations = 0;
        
        vector<int> course_count(num_courses, 0);
        for (const auto& a : sol.assignments) {
            course_count[a.course]++;
        }
        for (int c : course_count) {
            if (c != 1) violations++;
        }
        
        for (int l = 0; l < num_lecturers; l++) {
            for (int t = 0; t < num_timeslots; t++) {
                int count = 0;
                for (const auto& a : sol.assignments) {
                    if (courses[a.course].lecturer == l && 
                        check_overlap(a.timeslot, t, courses[a.course].duration)) {
                        count++;
                    }
                }
                if (count > 1) violations += count - 1;
            }
        }
        
        for (int g = 0; g < num_groups; g++) {
            for (int t = 0; t < num_timeslots; t++) {
                int count = 0;
                for (const auto& a : sol.assignments) {
                    if (courses[a.course].groups.count(g) && 
                        check_overlap(a.timeslot, t, courses[a.course].duration)) {
                        count++;
                    }
                }
                if (count > 1) violations += count - 1;
            }
        }
        
        for (int r = 0; r < num_rooms; r++) {
            for (int t = 0; t < num_timeslots; t++) {
                int count = 0;
                for (const auto& a : sol.assignments) {
                    if (a.room == r && 
                        check_overlap(a.timeslot, t, courses[a.course].duration)) {
                        count++;
                    }
                }
                if (count > 1) violations += count - 1;
            }
        }
        
        for (const auto& a : sol.assignments) {
            int total_students = 0;
            for (int g : courses[a.course].groups) {
                total_students += group_sizes[g];
            }
            if (total_students > rooms[a.room].capacity) {
                violations++;
            }
        }
        
        return violations;
    }
    
    int calculate_gaps(const Solution& sol) const {
        int total_gaps = 0;
        
        for (int g = 0; g < num_groups; g++) {
            for (int d = 0; d < num_days; d++) {
                set<int> periods;
                for (const auto& a : sol.assignments) {
                    if (courses[a.course].groups.count(g)) {
                        int day = a.timeslot / periods_per_day;
                        int period = a.timeslot % periods_per_day;
                        if (day == d) {
                            periods.insert(period);
                        }
                    }
                }
                if (!periods.empty()) {
                    int first = *periods.begin();
                    int last = *periods.rbegin();
                    int gaps = last - first + 1 - periods.size();
                    total_gaps += gaps;
                }
            }
        }
        
        return total_gaps;
    }
    
    int calculate_undesirable(const Solution& sol) const {
        int penalty = 0;
        
        for (const auto& a : sol.assignments) {
            int lec = courses[a.course].lecturer;
            if (lecturer_undesirable[lec].count(a.timeslot)) {
                penalty++;
            }
            
            for (int g : courses[a.course].groups) {
                if (group_undesirable[g].count(a.timeslot)) {
                    penalty++;
                }
            }
        }
        
        return penalty;
    }
    
    double evaluate(Solution& sol) const {
        sol.hard_violations = count_hard_violations(sol);
        sol.gap_penalty = calculate_gaps(sol);
        sol.undesirable_penalty = calculate_undesirable(sol);
        sol.cost = alpha * sol.hard_violations + beta * sol.gap_penalty + gamma * sol.undesirable_penalty;
        return sol.cost;
    }
};

class ACO {
private:
    UCTPInstance& instance;
    int num_ants;
    int max_iterations;
    double evaporation_rate;
    double pheromone_importance;
    double heuristic_importance;
    double initial_pheromone;
    
    vector<vector<vector<double>>> pheromone;
    vector<vector<vector<double>>> heuristic_cache;
    
    mt19937 gen;
    
    // Track iteration history
    struct IterationData {
        int iteration;
        double best_cost;
        int best_hard_violations;
        int best_gap_penalty;
        int best_undesirable_penalty;
        double avg_cost;
        double worst_cost;
    };
    vector<IterationData> iteration_history;
    
public:
    ACO(UCTPInstance& inst, int ants = 20, int iters = 100) 
        : instance(inst), num_ants(ants), max_iterations(iters),
          evaporation_rate(0.1), pheromone_importance(1.0), 
          heuristic_importance(2.0), initial_pheromone(1.0) {
        
        random_device rd;
        gen.seed(rd());
        
        pheromone.resize(instance.num_courses);
        heuristic_cache.resize(instance.num_courses);
        for (int c = 0; c < instance.num_courses; c++) {
            pheromone[c].resize(instance.num_rooms);
            heuristic_cache[c].resize(instance.num_rooms);
            for (int r = 0; r < instance.num_rooms; r++) {
                pheromone[c][r].resize(instance.num_timeslots, initial_pheromone);
                heuristic_cache[c][r].resize(instance.num_timeslots);
                for (int t = 0; t < instance.num_timeslots; t++) {
                    heuristic_cache[c][r][t] = calculate_heuristic(c, r, t);
                }
            }
        }
    }
    
    double calculate_heuristic(int course, int room, int timeslot) {
        double h = 1.0;
        
        int total_students = 0;
        for (int g : instance.courses[course].groups) {
            total_students += instance.group_sizes[g];
        }
        double capacity_ratio = (double)total_students / instance.rooms[room].capacity;
        if (capacity_ratio <= 1.0) {
            h *= (1.0 / (0.1 + abs(1.0 - capacity_ratio)));
        } else {
            h *= 0.01;
        }
        
        int lec = instance.courses[course].lecturer;
        if (instance.lecturer_undesirable[lec].count(timeslot)) {
            h *= 0.5;
        }
        for (int g : instance.courses[course].groups) {
            if (instance.group_undesirable[g].count(timeslot)) {
                h *= 0.5;
            }
        }
        
        return h;
    }
    
    Solution construct_solution() {
        Solution sol;
        vector<int> course_order(instance.num_courses);
        for (int i = 0; i < instance.num_courses; i++) course_order[i] = i;
        shuffle(course_order.begin(), course_order.end(), gen);
        
        for (int c : course_order) {
            vector<double> probabilities;
            vector<pair<int, int>> choices;
            
            double sum = 0.0;
            for (int r = 0; r < instance.num_rooms; r++) {
                for (int t = 0; t < instance.num_timeslots; t++) {
                    double tau = pheromone[c][r][t];
                    double eta = heuristic_cache[c][r][t];
                    double prob = pow(tau, pheromone_importance) * pow(eta, heuristic_importance);
                    probabilities.push_back(prob);
                    choices.push_back({r, t});
                    sum += prob;
                }
            }
            
            uniform_real_distribution<> dist(0.0, sum);
            double rand_val = dist(gen);
            double cumulative = 0.0;
            int selected = 0;
            for (size_t i = 0; i < probabilities.size(); i++) {
                cumulative += probabilities[i];
                if (cumulative >= rand_val) {
                    selected = i;
                    break;
                }
            }
            
            Assignment a;
            a.course = c;
            a.room = choices[selected].first;
            a.timeslot = choices[selected].second;
            sol.assignments.push_back(a);
        }
        
        instance.evaluate(sol);
        return sol;
    }
    
    void update_pheromones(const vector<Solution>& solutions, const Solution& best_global) {
        for (int c = 0; c < instance.num_courses; c++) {
            for (int r = 0; r < instance.num_rooms; r++) {
                for (int t = 0; t < instance.num_timeslots; t++) {
                    pheromone[c][r][t] *= (1.0 - evaporation_rate);
                }
            }
        }
        
        vector<Solution> sorted_sols = solutions;
        sort(sorted_sols.begin(), sorted_sols.end(), 
             [](const Solution& a, const Solution& b) { return a.cost < b.cost; });
        
        int elite_count = min(3, (int)sorted_sols.size());
        for (int e = 0; e < elite_count; e++) {
            double deposit = (elite_count - e) / (1.0 + sorted_sols[e].cost);
            for (const auto& a : sorted_sols[e].assignments) {
                pheromone[a.course][a.room][a.timeslot] += deposit;
            }
        }
        
        double best_deposit = 5.0 / (1.0 + best_global.cost);
        for (const auto& a : best_global.assignments) {
            pheromone[a.course][a.room][a.timeslot] += best_deposit;
        }
    }
    
    Solution solve() {
        Solution best_global;
        best_global.cost = numeric_limits<double>::max();
        
        for (int iter = 0; iter < max_iterations; iter++) {
            vector<Solution> iteration_solutions;
            Solution best_iteration;
            best_iteration.cost = numeric_limits<double>::max();
            
            for (int ant = 0; ant < num_ants; ant++) {
                Solution sol = construct_solution();
                iteration_solutions.push_back(sol);
                
                if (sol.cost < best_iteration.cost) {
                    best_iteration = sol;
                }
            }
            
            if (best_iteration.cost < best_global.cost) {
                best_global = best_iteration;
                cout << "Iteration " << iter << ": Cost = " << best_global.cost 
                     << " (HC: " << best_global.hard_violations 
                     << ", Gaps: " << best_global.gap_penalty 
                     << ", Undes: " << best_global.undesirable_penalty << ")" << endl;
            }
            
            // Record iteration statistics
            IterationData data;
            data.iteration = iter;
            data.best_cost = best_global.cost;
            data.best_hard_violations = best_global.hard_violations;
            data.best_gap_penalty = best_global.gap_penalty;
            data.best_undesirable_penalty = best_global.undesirable_penalty;
            
            // Calculate average and worst for this iteration
            double sum = 0.0;
            double worst = 0.0;
            for (const auto& sol : iteration_solutions) {
                sum += sol.cost;
                if (sol.cost > worst) worst = sol.cost;
            }
            data.avg_cost = sum / iteration_solutions.size();
            data.worst_cost = worst;
            
            iteration_history.push_back(data);
            
            update_pheromones(iteration_solutions, best_global);
        }
        
        return best_global;
    }
    
    void save_iteration_history(const string& filename) const {
        ofstream out(filename);
        out << "Iteration,BestCost,BestHardViolations,BestGapPenalty,BestUndesirablePenalty,AvgCost,WorstCost\n";
        for (const auto& data : iteration_history) {
            out << data.iteration << ","
                << data.best_cost << ","
                << data.best_hard_violations << ","
                << data.best_gap_penalty << ","
                << data.best_undesirable_penalty << ","
                << data.avg_cost << ","
                << data.worst_cost << "\n";
        }
        out.close();
    }
};

void save_solution(const string& filename, const Solution& sol, const UCTPInstance& inst, double runtime) {
    ofstream out(filename);
    out << "Course,Room,Day,Period,Timeslot\n";
    for (const auto& a : sol.assignments) {
        int day = a.timeslot / inst.periods_per_day;
        int period = a.timeslot % inst.periods_per_day;
        out << a.course << "," << a.room << "," << day << "," << period << "," << a.timeslot << "\n";
    }
    out.close();
    
    ofstream stats(filename + "_stats.txt");
    stats << "Runtime: " << runtime << " seconds\n";
    stats << "Total Cost: " << sol.cost << "\n";
    stats << "Hard Violations: " << sol.hard_violations << "\n";
    stats << "Gap Penalty: " << sol.gap_penalty << "\n";
    stats << "Undesirable Penalty: " << sol.undesirable_penalty << "\n";
    stats.close();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <dataset_id> [num_ants] [max_iterations]" << endl;
        cout << "Example: " << argv[0] << " 1 20 100" << endl;
        return 1;
    }
    
    int dataset_id = atoi(argv[1]);
    int num_ants = (argc > 2) ? atoi(argv[2]) : 20;
    int max_iterations = (argc > 3) ? atoi(argv[3]) : 100;
    
    string prefix = "dataset_" + to_string(dataset_id);
    
    cout << "Loading dataset " << dataset_id << "..." << endl;
    UCTPInstance instance;
    instance.load_from_csv(prefix);
    
    cout << "\nRunning ACO with " << num_ants << " ants, " << max_iterations << " iterations..." << endl;
    
    auto start = chrono::high_resolution_clock::now();
    ACO aco(instance, num_ants, max_iterations);
    Solution best = aco.solve();
    auto end = chrono::high_resolution_clock::now();
    double runtime = chrono::duration<double>(end - start).count();
    
    cout << "\n=== FINAL SOLUTION ===" << endl;
    cout << "Cost: " << best.cost << endl;
    cout << "Hard Violations: " << best.hard_violations << endl;
    cout << "Gap Penalty: " << best.gap_penalty << endl;
    cout << "Undesirable Penalty: " << best.undesirable_penalty << endl;
    cout << "Runtime: " << runtime << " seconds" << endl;
    
    string output_file = prefix + "_solution.csv";
    save_solution(output_file, best, instance, runtime);
    cout << "\nSolution saved to " << output_file << endl;
    
    string iteration_file = prefix + "_iterations.csv";
    aco.save_iteration_history(iteration_file);
    cout << "Iteration history saved to " << iteration_file << endl;
    
    return 0;
}
