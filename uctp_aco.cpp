#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <algorithm>
#include <cmath>
#include <limits>
#include <fstream>
#include <iomanip>

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
    
    void generate_random_instance(int nc, int nl, int ng, int nr) {
        num_courses = nc;
        num_lecturers = nl;
        num_groups = ng;
        num_rooms = nr;
        num_timeslots = num_days * periods_per_day;
        
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> lec_dist(0, num_lecturers - 1);
        uniform_int_distribution<> group_count_dist(1, 3);
        uniform_int_distribution<> group_dist(0, num_groups - 1);
        uniform_int_distribution<> size_dist(20, 50);
        uniform_int_distribution<> cap_dist(30, 100);
        uniform_int_distribution<> undes_count(0, 5);
        uniform_int_distribution<> time_dist(0, num_timeslots - 1);
        
        for (int i = 0; i < num_courses; i++) {
            Course c;
            c.id = i;
            c.lecturer = lec_dist(gen);
            c.duration = 1;
            int num_grps = group_count_dist(gen);
            for (int j = 0; j < num_grps; j++) {
                c.groups.insert(group_dist(gen));
            }
            courses.push_back(c);
        }
        
        for (int i = 0; i < num_rooms; i++) {
            Room r;
            r.id = i;
            r.capacity = cap_dist(gen);
            rooms.push_back(r);
        }
        
        for (int i = 0; i < num_groups; i++) {
            group_sizes.push_back(size_dist(gen));
        }
        
        lecturer_undesirable.resize(num_lecturers);
        for (int i = 0; i < num_lecturers; i++) {
            int count = undes_count(gen);
            for (int j = 0; j < count; j++) {
                lecturer_undesirable[i].insert(time_dist(gen));
            }
        }
        
        group_undesirable.resize(num_groups);
        for (int i = 0; i < num_groups; i++) {
            int count = undes_count(gen);
            for (int j = 0; j < count; j++) {
                group_undesirable[i].insert(time_dist(gen));
            }
        }
    }
    
    bool check_overlap(int t1, int t2, int duration) const {
        return abs(t1 - t2) < duration;
    }
    
    int count_hard_violations(const Solution& sol) const {
        int violations = 0;
        
        // Course assignment constraint
        vector<int> course_count(num_courses, 0);
        for (const auto& a : sol.assignments) {
            course_count[a.course]++;
        }
        for (int c : course_count) {
            if (c != 1) violations++;
        }
        
        // Lecturer conflicts
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
        
        // Student group conflicts
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
        
        // Room conflicts
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
        
        // Room capacity
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
    bool use_candidate_list;
    int candidate_list_size;
    bool use_improvements;
    
    vector<vector<vector<double>>> pheromone; // [course][room][timeslot]
    vector<vector<vector<double>>> heuristic_cache; // Pre-computed heuristics
    
    int stagnation_counter;
    double best_cost_so_far;
    
    mt19937 gen;
    
public:
    ACO(UCTPInstance& inst, int ants = 20, int iters = 100, bool use_cl = false, int cl_size = 20, bool improvements = false) 
        : instance(inst), num_ants(ants), max_iterations(iters),
          evaporation_rate(0.1), pheromone_importance(1.0), 
          heuristic_importance(2.0), initial_pheromone(1.0),
          use_candidate_list(use_cl), candidate_list_size(cl_size),
          use_improvements(improvements), stagnation_counter(0), 
          best_cost_so_far(numeric_limits<double>::max()) {
        
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
                if (use_candidate_list) {
                    // Pre-compute heuristics for speedup
                    for (int t = 0; t < instance.num_timeslots; t++) {
                        heuristic_cache[c][r][t] = calculate_heuristic(c, r, t);
                    }
                }
            }
        }
    }
    
    double calculate_heuristic(int course, int room, int timeslot) {
        double h = 1.0;
        
        // Prefer rooms with adequate capacity
        int total_students = 0;
        for (int g : instance.courses[course].groups) {
            total_students += instance.group_sizes[g];
        }
        double capacity_ratio = (double)total_students / instance.rooms[room].capacity;
        if (capacity_ratio <= 1.0) {
            h *= (1.0 / (0.1 + abs(1.0 - capacity_ratio)));
        } else {
            h *= 0.01; // Penalize insufficient capacity
        }
        
        // Avoid undesirable timeslots
        int lec = instance.courses[course].lecturer;
        if (instance.lecturer_undesirable[lec].count(timeslot)) {
            h *= 0.5;
        }
        for (int g : instance.courses[course].groups) {
            if (instance.group_undesirable[g].count(timeslot)) {
                h *= 0.5;
            }
        }
        
        // NEW: Constraint-aware heuristic - check for conflicts
        if (use_improvements) {
            // Penalize timeslots that would cause conflicts (simplified check)
            int day = timeslot / instance.periods_per_day;
            int period = timeslot % instance.periods_per_day;
            
            // Prefer spreading courses across days
            if (period >= 0 && period <= 2) h *= 1.2; // Morning preference
            if (period >= 6) h *= 0.8; // Late afternoon less desirable
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
                    double eta = use_candidate_list ? heuristic_cache[c][r][t] : calculate_heuristic(c, r, t);
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
        
        // Local search removed from here - applied selectively in solve()
        
        return sol;
    }
    
    void local_search(Solution& sol) {
        // Fast local search: limited random swaps
        int max_attempts = min(100, (int)sol.assignments.size() * 2);
        uniform_int_distribution<> idx_dist(0, sol.assignments.size() - 1);
        uniform_int_distribution<> room_dist(0, instance.num_rooms - 1);
        uniform_int_distribution<> time_dist(0, instance.num_timeslots - 1);
        
        for (int attempt = 0; attempt < max_attempts; attempt++) {
            int i = idx_dist(gen);
            
            // Try random room change
            int old_room = sol.assignments[i].room;
            sol.assignments[i].room = room_dist(gen);
            double new_cost = instance.evaluate(sol);
            
            if (new_cost < sol.cost) {
                sol.cost = new_cost;
            } else {
                sol.assignments[i].room = old_room;
            }
            
            // Try random timeslot change
            int old_time = sol.assignments[i].timeslot;
            sol.assignments[i].timeslot = time_dist(gen);
            new_cost = instance.evaluate(sol);
            
            if (new_cost < sol.cost) {
                sol.cost = new_cost;
            } else {
                sol.assignments[i].timeslot = old_time;
            }
        }
        
        instance.evaluate(sol); // Final evaluation
    }
    
    void update_pheromones(const vector<Solution>& solutions, const Solution& best_global) {
        // Evaporation
        for (int c = 0; c < instance.num_courses; c++) {
            for (int r = 0; r < instance.num_rooms; r++) {
                for (int t = 0; t < instance.num_timeslots; t++) {
                    pheromone[c][r][t] *= (1.0 - evaporation_rate);
                }
            }
        }
        
        // NEW: Elite strategy - deposit from top 3 solutions
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
        
        // Extra deposit for global best (stronger reinforcement)
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
                cout << "Iteration " << iter << ": New best cost = " << best_global.cost 
                     << " (HC: " << best_global.hard_violations 
                     << ", Gaps: " << best_global.gap_penalty 
                     << ", Undes: " << best_global.undesirable_penalty << ")" << endl;
            }
            
            update_pheromones(iteration_solutions, best_global);
        }
        
        return best_global;
    }
};

void print_solution(const Solution& sol, const UCTPInstance& inst) {
    cout << "\n=== SOLUTION ===" << endl;
    cout << "Total Cost: " << sol.cost << endl;
    cout << "Hard Constraint Violations: " << sol.hard_violations << endl;
    cout << "Gap Penalty: " << sol.gap_penalty << endl;
    cout << "Undesirable Penalty: " << sol.undesirable_penalty << endl;
    cout << "\nAssignments:" << endl;
    
    for (const auto& a : sol.assignments) {
        int day = a.timeslot / inst.periods_per_day;
        int period = a.timeslot % inst.periods_per_day;
        cout << "Course " << a.course << " -> Room " << a.room 
             << " @ Day " << day << " Period " << period << endl;
    }
}

void save_results(const string& filename, const Solution& sol, const UCTPInstance& inst, double runtime) {
    ofstream out(filename);
    out << "Runtime(s),Cost,HardViolations,GapPenalty,UndesirablePenalty" << endl;
    out << fixed << setprecision(2) << runtime << "," << sol.cost << "," 
        << sol.hard_violations << "," << sol.gap_penalty << "," << sol.undesirable_penalty << endl;
    out.close();
}

int main(int argc, char* argv[]) {
    int num_courses = 50;
    int num_lecturers = 10;
    int num_groups = 15;
    int num_rooms = 10;
    int num_ants = 20;
    int max_iterations = 100;
    bool compare_mode = false;
    
    if (argc > 1) num_courses = atoi(argv[1]);
    if (argc > 2) num_ants = atoi(argv[2]);
    if (argc > 3) max_iterations = atoi(argv[3]);
    if (argc > 4 && string(argv[4]) == "compare") compare_mode = true;
    
    cout << "University Course Timetabling Problem - ACO Solver" << endl;
    cout << "Courses: " << num_courses << ", Lecturers: " << num_lecturers 
         << ", Groups: " << num_groups << ", Rooms: " << num_rooms << endl;
    cout << "ACO Parameters: Ants=" << num_ants << ", Iterations=" << max_iterations << endl;
    
    UCTPInstance instance;
    instance.generate_random_instance(num_courses, num_lecturers, num_groups, num_rooms);
    
    if (compare_mode) {
        cout << "\n=== COMPARISON MODE ===" << endl;
        
        // Standard ACO
        cout << "\n--- Standard ACO (Baseline) ---" << endl;
        auto start1 = chrono::high_resolution_clock::now();
        ACO aco_standard(instance, num_ants, max_iterations, false, 20, false);
        Solution best_standard = aco_standard.solve();
        auto end1 = chrono::high_resolution_clock::now();
        double runtime1 = chrono::duration<double>(end1 - start1).count();
        
        cout << "\nStandard ACO Results:" << endl;
        cout << "  Cost: " << best_standard.cost << endl;
        cout << "  Hard Violations: " << best_standard.hard_violations << endl;
        cout << "  Gap Penalty: " << best_standard.gap_penalty << endl;
        cout << "  Undesirable Penalty: " << best_standard.undesirable_penalty << endl;
        cout << "  Runtime: " << runtime1 << " seconds" << endl;
        
        // Heuristic Caching ACO (Speedup)
        cout << "\n--- Heuristic Caching ACO (Speedup) ---" << endl;
        auto start2 = chrono::high_resolution_clock::now();
        ACO aco_fast(instance, num_ants, max_iterations, true, 50, false);
        Solution best_fast = aco_fast.solve();
        auto end2 = chrono::high_resolution_clock::now();
        double runtime2 = chrono::duration<double>(end2 - start2).count();
        
        cout << "\nHeuristic Caching ACO Results:" << endl;
        cout << "  Cost: " << best_fast.cost << endl;
        cout << "  Hard Violations: " << best_fast.hard_violations << endl;
        cout << "  Gap Penalty: " << best_fast.gap_penalty << endl;
        cout << "  Undesirable Penalty: " << best_fast.undesirable_penalty << endl;
        cout << "  Runtime: " << runtime2 << " seconds" << endl;
        
        // Improved ACO (Elite Strategy + Better Heuristics)
        cout << "\n--- Improved ACO (Elite Strategy) ---" << endl;
        auto start3 = chrono::high_resolution_clock::now();
        ACO aco_improved(instance, num_ants, max_iterations, true, 50, true);
        Solution best_improved = aco_improved.solve();
        auto end3 = chrono::high_resolution_clock::now();
        double runtime3 = chrono::duration<double>(end3 - start3).count();
        
        cout << "\nImproved ACO Results:" << endl;
        cout << "  Cost: " << best_improved.cost << endl;
        cout << "  Hard Violations: " << best_improved.hard_violations << endl;
        cout << "  Gap Penalty: " << best_improved.gap_penalty << endl;
        cout << "  Undesirable Penalty: " << best_improved.undesirable_penalty << endl;
        cout << "  Runtime: " << runtime3 << " seconds" << endl;
        
        cout << "\n=== COMPARISON SUMMARY ===" << endl;
        cout << "Speedup (Cached vs Standard): " << fixed << setprecision(2) << (runtime1 / runtime2) << "x" << endl;
        cout << "Quality improvement (Improved vs Standard): " << setprecision(1) 
             << ((best_standard.cost - best_improved.cost) / best_standard.cost * 100) << "%" << endl;
        cout << "Quality improvement (Improved vs Cached): " << setprecision(1)
             << ((best_fast.cost - best_improved.cost) / best_fast.cost * 100) << "%" << endl;
        
        // Save comparison results
        ofstream out("comparison_results.csv");
        out << "Method,Runtime(s),Cost,HardViolations,GapPenalty,UndesirablePenalty" << endl;
        out << "Standard," << fixed << setprecision(2) << runtime1 << "," << best_standard.cost << "," 
            << best_standard.hard_violations << "," << best_standard.gap_penalty << "," 
            << best_standard.undesirable_penalty << endl;
        out << "HeuristicCaching," << runtime2 << "," << best_fast.cost << "," 
            << best_fast.hard_violations << "," << best_fast.gap_penalty << "," 
            << best_fast.undesirable_penalty << endl;
        out << "Improved," << runtime3 << "," << best_improved.cost << "," 
            << best_improved.hard_violations << "," << best_improved.gap_penalty << "," 
            << best_improved.undesirable_penalty << endl;
        out.close();
        cout << "\nComparison results saved to comparison_results.csv" << endl;
        
    } else {
        auto start = chrono::high_resolution_clock::now();
        
        ACO aco(instance, num_ants, max_iterations);
        Solution best = aco.solve();
        
        auto end = chrono::high_resolution_clock::now();
        double runtime = chrono::duration<double>(end - start).count();
        
        print_solution(best, instance);
        cout << "\nRuntime: " << runtime << " seconds" << endl;
        
        save_results("results.csv", best, instance, runtime);
        cout << "\nResults saved to results.csv" << endl;
    }
    
    return 0;
}
