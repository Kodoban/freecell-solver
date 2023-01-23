// Τεχνητή Νοημοσύνη - Εργασία 1: Freecell

/*
	File saved in UTF-8 encoding due to comments in Greek
	Please change the encoding settings in your text editor/IDE if non-English comments are not displayed properly
*/

/*
	Για λόγους συνοχής και ευκολότερης κατανόησης, τα σχόλια γραμμένα στα ελληνικά αφορούν είτε πρωτότυπα κομμάτια κώδικα, 
	είτε κομμάτια κώδικα του puzzle.c τα οποία έχουν προσαρμοστεί για το πρόγραμμα αυτό,
	ενώ τα σχόλια στα αγγλικά αφορούν μη αλλαγμένα κομμάτια κώδικα από το αρχικό αρχείο puzzle.c (π.χ. η δομή frontier_node).

	Επιπλέον, οι συναρτήσεις οι οποίες δεν έχουν «πειραχθεί» από το αρχείο puzzle.c (αλλά αξιοποιούνται κατά την εκτέλεση) 
	έχουν τοποθετηθεί στο βάθος των χρησιμοποιούμενων συναρτήσεων (από γραμμή 1336).
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Σταθερές για τις 4 φυλές φύλλων
#define HEARTS 0
#define DIAMONDS 1
#define SPADES 2
#define CLUBS 3

//Σταθερές για τα 2 χρώματα φύλλων
#define RED 0
#define BLACK 1

//Σταθερές για τα 4 είδη επιτρεπτών κινήσεων
//Οι 3 πρώτες σταθερές αξιοποιούνται επίσης για να περιγράψουν την περιοχή προέλευσης για την ευκολότερη καταγραφή
//της αλλαγής της κατάστασης του ταμπλό (καθώς η μετακίνηση από άδεια στοίβα δεν είναι δυνατή)
//(Π.χ. για την κίνηση από στοίβα χρησιμοποιείται η σταθερά STACK)
#define STACK 0
#define FREECELL 1
#define FOUNDATION 2
#define NEW_STACK 3

//Σταθερές για το πλήθος των θέσεων κάθε τομέα του ταμπλό
#define MAIN_STACKS_NUM 8
#define FREECELLS_NUM 4
#define FOUNDATIONS_NUM 4

//Σταθερές για τις 4 μεθόδους αναζήτησης
#define BREADTH 1
#define DEPTH	2
#define BEST 	3
#define A_STAR	4

//Σταθερά για το πλήθος φύλλων ανά φυλή
#define N 13


//Δομή για κάρτες
//Τα χαρακτηριστικά κάθε κάρτας είναι το χρώμα, η φυλή και η τιμή της
typedef struct {
	int color;
	int suit;
	int value;
} card_t;

//Δομή για την κατάσταση του ταμπλό
//Η κατάσταση περιέχει τρεις πίνακες, έναν για κάθε τομέα του ταμπλό, στους οποίους καταγράφονται οι κάρτες που περιέχονται στον κάθε τομέα.
//Οι πίνακες των κυρίως στοιβών (main_stack_cards) και των foundations (foundation_cards) είναι 2Δ πίνακες καρτών,
//καθώς η κάθε στήλη τους (η οποία αντιστοιχεί στις γραμμές του πίνακα) ακολουθείται από ένα πλήθος καρτών,
//σε αντίθεση με τη στοίβα των freecell (freecell_cards) η οποία είναι 1Δ πίνακας-διάνυσμα καρτών.
//Τέλος, η κατάσταση περιέχει τα μεγέθη των πινάκων, δηλαδή το πλήθος των θέσεων τους που έχει καταληφθεί.
//Για τους 2Δ πίνακες, καταγράφεται το μέγεθος της κάθε στήλης τους σε ένα διάνυσμα ακεραίων,
//Ενώ για τα freecells, καταγράφονται οι κατειλημμένες θέσεις ως απλός ακέραιος
typedef struct {

	//Πίνακας κυρίως στοιβών
	//Διαστάσεις: MAIN_STACKS_NUM (8 σε μια πλήρη παρτίδα) x 20
	//Το πλήθος των στηλών προκύπτει ως εξής:
	//Για μία πλήρη παρτίδα Κυψελών (με N=13), οι κάρτες στην αρχική τους κατάσταση μοιράζονται σε τέσσερεις στήλες των 8 καρτών, και τέσσερις των 7 καρτών.
	//Έστω ότι σε μία από τις στήλες των 8 καρτών, το 8ο χαρτί είναι Ρήγας.
	//Θεωρητικά, λοιπόν, πάνω από το Ρήγα μπορεί να τοποθετηθεί μία στοίβα 12 καρτών αξίας 12 (Ντάμα) έως 1.
	//Αυτή η στήλη έχει μήκος 20, και δεν είναι δυνατή η επέκτασή της, καθώς δεν υπάρχει κάρτα μικρότερου του άσσου.
	//Συνεπώς, για μία πλήρη παρτίδα Κυψελών, το μέγιστο μήκος μίας κύριας στοίβας είναι 20 φύλλα.
	card_t main_stack_cards[MAIN_STACKS_NUM][20];
	int main_stack_sizes[MAIN_STACKS_NUM]; //Διάνυσμα καταγραφής μήκους κάθε κύριας στοίβας

	//Πίνακας των freecells
	//Διαστάσεις: FREECELLS_NUM (4 σε μια πλήρη παρτίδα)
	card_t freecell_cards[FREECELLS_NUM];
	int freecell_size; //Κατειλημμένες θέσεις freecell

	//Πίνακας των freecells
	//Διαστάσεις: FOUNDATIONS_NUM (4 σε μια πλήρη παρτίδα) x N (13 σε μία πλήρη παρτίδα)
	//Η κάθε γραμμή του πίνακα foundation_cards αντιστοιχεί σε μία φυλή
	//Οι αριθμοδείκτες 0-3 του πίνακα αντιστοιχούν στις σταθερές 0-3 των φυλών,
	//και συνεπώς η θέση κάθε φυλής είναι εκ των προτέρων καθορισμένη για την ευκολότερη διαχείρισή τους και πρόσβαση σε αυτές.
	//Το πλήθος Ν των στήλων του πίνακα δηλώνει το μέγιστο «ύψος» των φυλών στα foundations
	//(Π.χ. για N=8, κάθε φυλή έχει φύλλα τιμής 1-8 [0-7 βάσει του generator.c] στη διάθεσή της για να τοποθετηθούν στις 8 θέσεις τους)
	card_t foundation_cards[FOUNDATIONS_NUM][N];
	//Διάνυσμα καταγραφής μήκους κάθε κύριας στοίβας. Από τον πίνακα αυτό εξάγονται και οι κορυφές των στοιβών,
	//καθώς το μήκος της στοίβας (έστω L) είναι ενδεικτικό της κάρτας στην κορυφή της στοίβας
	//(αν L=7 τότε η τιμή της κορυφαίας κάρτας είναι 7 [για χαμηλότερη τιμή 1] ή 6 (δηλαδή L-1) [για χαμηλότερη τιμή 0])
	int foundation_card_sizes[FOUNDATIONS_NUM]; //Διάνυσμα καταγραφής μήκους κάθε foundation

} board_state_t;

//Δομή για τα στοιχεία της κίνησης που πραγματοποιείται στον κόμβο
typedef struct {
	int move_from; //Προέλευση κίνησης (STACK, FREECELL, FOUNDATION)
	int move_type; //Τύπος κίνησης/Προορισμός κίνησης (STACK, FREECELL, FOUNDATION, NEW_STACK)
	card_t *card_moved; //Δείκτης προς την κάρτα που μετακινήθηκε
	card_t *card_below; //Δείκτης προς την κάρτα πάνω από την οποία μετακινήθηκε η card_moved (μόνο για move_type==STACK, αλλιώς NULL)
} card_movement_t;

//Δομή για τα στοιχεία της κίνησης του βήματος που πραγματοποιήθηκε σε κόμβο του κλαδιού-λύσης
//Χρησιμοποιείται κατά την συγγραφή του αρχείου εξόδου
typedef struct {
	int move_type;
	card_t *card_moved;
	card_t *card_below; //Χρησιμοποιείται μόνο για τη μετακίνηση σε στήλη του κυρίως ταμπλό
} solution_step_t;

//Δομή για τα στοιχεία του κάθε κόμβου
struct tree_node
{
    board_state_t board_state; //Κατάσταση του ταμπλό μετά την κίνηση που πραγματοποιήθηκε
	card_movement_t card_movement; //Η κίνηση που πραγματοποιήθηκε
	int h;				// the value of the heuristic function for this node
	int g;				// the depth of this node wrt the root of the search tree
	int f;				// f=0 or f=h or f=h+g, depending on the search algorithm used.
	struct tree_node *parent;	// pointer to the parrent node (NULL for the root).
};

// A node of the frontier. Frontier is kept as a double-linked list,
// for efficiency reasons for the breadth-first search algorithm.
struct frontier_node
{
	struct tree_node *n;			// pointer to a search-tree node
	struct frontier_node *previous;	// pointer to the previous frontier node
	struct frontier_node *next;		// pointer to the next frontier node
};

struct frontier_node *frontier_head=NULL;	// The one end of the frontier
struct frontier_node *frontier_tail=NULL;	// The other end of the frontier

clock_t t1;				// Start time of the search algorithm
clock_t t2;				// End time of the search algorithm
#define TIMEOUT		60		// Program terminates after TIMEOUT secs

int solution_length;		// The length of the solution table.
int loops= 4*N;				// Το πλήθος των καρτών που καλείται να διαβάσει το πρόγραμμα από το αρχείο εισόδου
solution_step_t *solution; 	// Μεταβλητή στην οποία αποθηκεύονται τα βήματα της λύσης

//Αρχή forward declaration
void syntax_message();
int get_method(char* s);
int add_frontier_front(struct tree_node *node);
int add_frontier_back(struct tree_node *node);
int add_frontier_in_order(struct tree_node *node);

int read_board_state(char* filename, board_state_t *board_state);
void initialize_main_stack_state(board_state_t *board_state);
void initialize_foundations_state(board_state_t *board_state);
void initialize_freecell_state(board_state_t *board_state);
void initialize_search(board_state_t board_state, int method);

int heuristic(board_state_t board_state);
int min_moves_to_free_next_foundation_card(board_state_t board_state, int *min_moves_suit);

struct tree_node *search(int method);
int is_solution(struct tree_node *node);

int find_add_children(struct tree_node *current_node, int method);
int check_add_card_to_stack(struct tree_node *current_node, card_t *curr_i_card, int i, int move_from, int method);
int check_add_card_to_foundation(struct tree_node *current_node, card_t *curr_i_card, int i, int move_from, int method);
int check_add_card_to_freecell(struct tree_node *current_node, card_t *curr_i_card, int i, int move_from, int method);
int add_new_child(struct tree_node *parent, int source_i, int dest_i, int move_type, int move_from, card_t* card_moved, card_t* card_below, int method);

void copy_board_state(struct tree_node *child, board_state_t parent_board_state);
void copy_main_stack(struct tree_node *child, board_state_t parent_board_state);
void copy_foundations(struct tree_node *child, board_state_t parent_board_state);
void copy_freecells(struct tree_node *child, board_state_t parent_board_state);

void move_card(struct tree_node *child, int source_i, int dest_i, int dest_i_new_top);
int check_with_parents(struct tree_node *new_node);
int identical_board_states(struct tree_node *new_node, struct tree_node *ascendant_node);
int equal_freecells(struct tree_node *new_node, struct tree_node *ascendant_node);
int equal_foundations(struct tree_node *new_node, struct tree_node *ascendant_node);
int equal_main_stacks(struct tree_node *new_node, struct tree_node *ascendant_node);
void calc_child_node_value(struct tree_node *child, int method);

void extract_solution(struct tree_node *solution_node);
void write_solution_to_file(char* filename);
void solution_step_text(int step, FILE *fout, int is_stack_to_stack_move);

//Συναρτήσεις εναλλακτικής ευρετικής συνάρτησης
// int find_highest_value_card_opportunities(board_state_t board_state, int *empty_cell_num);
// int bonus_score_for_complement_cards(board_state_t board_state, card_t curr_stack_top_card);

//ΣΥΝΑΡΤΗΣΕΙΣ DEBUG
void display_board_state(struct tree_node *current_node);
void display_card(card_t card);

//Τέλος forward declaration

int main(int argc, char** argv)
{
	int err;
	struct tree_node *solution_node;
	board_state_t board_state;		// The initial board_state read from a file
	int method;				// The search algorithm that will be used to solve the puzzle.

	if (argc!=4)
	{
		printf("Wrong number of arguments. Use correct syntax:\n");
		syntax_message();
		return -1;
	}

	method=get_method(argv[1]);

	if (method<0)
	{
		printf("Wrong method. Use correct syntax:\n");
		syntax_message();
		return -1;
	}

	err=read_board_state(argv[2], &board_state);
	if (err<0)
		return -1;

	printf("Solving %s using %s...\n",argv[2],argv[1]);
	t1=clock();

	initialize_search(board_state, method);

	solution_node = search(method);			// The main call

	t2=clock();

	if (solution_node!=NULL)
		extract_solution(solution_node);
	else
		printf("No solution found.\n");

	if (solution!=NULL)
	{
		printf("Solution found! (%d steps)\n",solution_length);
		printf("Time spent: %f secs\n",((float) t2-t1)/CLOCKS_PER_SEC);
		write_solution_to_file(argv[3]);
	}

	return 0;
}

// Η συνάρτηση εισάγει την αρχική κατάσταση της παρτίδας από το αρχείο εισόδου
// Αρχικά, αρχικοποιεί τη μεταβλητή board_state, στην οποία θα εισαχθεί η αρχική κατάσταση,
// και στη συνέχεια διαβάζει μία προς μία τις κάρτες της αρχικής κατάστασης, έως ότου φτάσει στο τέλος του αρχείου.
// Inputs:
//		char* filename	: Το όνομα του αρχείου που περιέχει την αρχική κατάσταση της παρτίδας
//		board_state_t *board_state: Μεταβλητή στην οποία καταχωρείται η αρχική κατάσταση της παρτίδας
// Output:
//		0 --> Successful read.
//		-1 --> Unsuccessful read
int read_board_state(char* filename, board_state_t *board_state)
{
	FILE *fin;
	int i,j,err;
	fin=fopen(filename, "r");
	card_t temp_card; //Προσωρινή μεταβλητή καταχώρισης στοιχείων της κάθε κάρτας που διαβάζεται από το αρχείο εισόδου
	char whitespace;

	if (fin==NULL)
	{
		#ifdef SHOW_COMMENTS
			printf("Cannot open file %s. Program terminates.\n",filename);
		#endif
		return -1;
	}

	//Αρχικοποίηση των τομέων του ταμπλό
	initialize_main_stack_state(board_state);
	initialize_freecell_state(board_state);
	initialize_foundations_state(board_state);

	//Γνωρίζοντας πως η αρχική κατάσταση του παιχνιδιού θα έχει κάρτες μόνο στις κύριες στοίβες,
	//ο εξωτερικός επαναληπτικός βρόχος, σε κάθε επανάληψη, διαβάζει τα περιεχόμενα της εκάστοτε στήλης, και τα εισάγει στην μεταβλητή board_state.
	//Ο εσωτερικός επαναληπτικός βρόχος και η μεταβλητή loops διασφαλίζουν πως θα διαβαστούν όλες οι κάρτες του αρχείου κατάλληλα,
	//χωρίς να φτάσει η fscanf στον χαρακτήρα EOF έχοντας παραλείψει κάποια κάρτα.
	for (i=0;i<MAIN_STACKS_NUM;i++)
	{
		//Ο εσωτερικός επαναληπτικός βρόχος διαβάζει μία προς μία τις κάρτες που εμφανίζονται στο αρχείου εισόδου.
		//Αρχικά, πριν την έναρξη του βρόχου αυτού, ορίζεται η τιμή της μεταβλητής whitespace ως ο χαρακτήρας του κενού.
		//Στο αρχείο εισόδου, κάθε κάρτα αποτελείται από ένα γράμμα και έναν αριθμό,
		//και ακολουθείται από έναν χαρακτήρα (κενού αν υπάρχει επόμενη κάρτα στη στήλη, newline αν είναι η κορυφαία κάρτα)
		//(Π.χ. H3 S2 H0 C1\n)
		//Η επανάληψη σταματάει μόλις εντοπιστεί ο χαρακτήρας newline, όπου και συνεχίζεται η εκτέλεση του εξωτερικού κόμβου
		//Μόλις η loops γίνει 0, έχουν διαβαστεί όλες οι κάρτες του αρχείου (πλήθους 4*Ν) χωρίς σφάλματα,
		//η ανάγνωση του αρχείου έχει ολοκληρωθεί επιτυχώς, έχει συμπληρωθεί και η τελευταία στοίβα, και συνεπώς μπορεί να κλείσει η ροή του αρχείου.
		whitespace=' ';
		for(j=0;whitespace!='\n';j++)
		{
		    char suit;

		    if (loops==0) break;

			err=fscanf(fin,"%c%d%c",&suit,&temp_card.value,&whitespace);
			if (err<3)
			{
				#ifdef SHOW_COMMENTS
					printf("Cannot read item [%d][%d] of the game state. Program terminates.\n",i,j);
				#endif
				fclose(fin);
				return -1;
			}

			//Καθώς οι φυλές των καρτών διαβάζονται ως γράμματα, το switch-case αντιστοιχίζει
			//τα γράμματα με τις αντίστοιχες αριθμητικές σταθερές των φυλών, ενώ ταυτόχρονα ορίζει και το χρώμα των καρτών
			switch (suit)
			{
			    case 'H':
			        temp_card.suit=HEARTS;
			        temp_card.color=RED;
					break;
				case 'D':
				    temp_card.suit=DIAMONDS;
				    temp_card.color=RED;
					break;
				case 'S':
				    temp_card.suit=SPADES;
				    temp_card.color=BLACK;
					break;
				case 'C':
				    temp_card.suit=CLUBS;
                    temp_card.color=BLACK;
					break;
			}

			//Η μεταβλητή temp_card αντίγραφεται στη θέση [i][j] των κύριων στοιβών, όπου και αντιστοιχεί,
			//και το μέγεθος της στοίβας i αυξάνεται κατά 1
            board_state->main_stack_cards[i][j]=temp_card;
			board_state->main_stack_sizes[i]++;

			//Με κάθε κάρτα που διαβάζεται επιτυχώς από το αρχείο εισόδου, η loops μειώνεται κατά 1
			loops--;
		}
	}
	fclose(fin);
	return 0;
}

//Αρχικοποίηση της κατάστασης των κύριων στοιβών
//Για κάθε στοίβα i, το μέγεθος της ορίζεται ως 0, ενώ κάθε κενή θέση λαμβάνει τιμή -1
// Inputs:
//		board_state_t *board_state: Μεταβλητή στην οποία αρχικοποιείται η κατάσταση του ταμπλό
// Output:
//		Κατ' αναφορά: Η αρχικοποιημένη κατάσταση των κύριων στοιβών του board_state
void initialize_main_stack_state(board_state_t *board_state)
{
	for (int i=0;i<MAIN_STACKS_NUM;i++)
	{
	    board_state->main_stack_sizes[i]=0;
	    for (int j=0;j<20;j++)
            board_state->main_stack_cards[i][j].value=-1;
	}
}

//Αρχικοποίηση της κατάστασης των freecells
//Το μέγεθος ορίζεται ως 0, ενώ κάθε κενή θέση λαμβάνει τιμή -1
// Inputs:
//		board_state_t *board_state: Μεταβλητή στην οποία αρχικοποιείται η κατάσταση του ταμπλό
// Output:
//		Κατ' αναφορά: Η αρχικοποιημένη κατάσταση των freecells του board_state
void initialize_freecell_state(board_state_t *board_state)
{
	board_state->freecell_size=0; //Κατειλημμένες θέσεις

	for (int i=0;i<FREECELLS_NUM;i++)
		board_state->freecell_cards[i].value=-1;
}

//Αρχικοποίηση της κατάστασης των foundations
//Για κάθε στοίβα i, το μέγεθος της ορίζεται ως 0, ενώ κάθε κενή θέση λαμβάνει τιμή -1
// Inputs:
//		board_state_t *board_state: Μεταβλητή στην οποία αρχικοποιείται η κατάσταση του ταμπλό
// Output:
//		Κατ' αναφορά: Η αρχικοποιημένη κατάσταση των foundations του board_state
void initialize_foundations_state(board_state_t *board_state)
{
    for (int i=0;i<FOUNDATIONS_NUM;i++)
	{
	    board_state->foundation_card_sizes[i]=0;
	    for (int j=0;j<N;j++)
            board_state->foundation_cards[i][j].value=-1;
	}
}

// This function initializes the search, i.e. it creates the root node of the search tree
// and the first node of the frontier.
void initialize_search(board_state_t board_state, int method)
{
	struct tree_node *root=NULL;	// the root of the search tree.

	// Initialize search tree
	root=(struct tree_node*) malloc(sizeof(struct tree_node));
	root->parent=NULL;
	root->card_movement.move_type=-1; //Καθώς η αρχική κατάσταση δεν έχει πρόγονο, δεν ορίζεται τύπος κίνησης

	root->board_state=board_state; //Αντιγραφή του αρχικού board_state

	root->g=0;
	root->h=heuristic(root->board_state);
	if (method==BEST)
		root->f=root->h;
	else if (method==A_STAR)
		root->f=root->g+root->h;
	else
		root->f=0;

	// Initialize frontier
	add_frontier_front(root);
}

//Επιστρέφει το αποτέλεσμα της ευρετικής συνάρτησης
//Σκοπεύει στην αποσυμφόρηση του ταμπλό από όσες περισσότερες κάρτες γίνεται. Αποτελείται από 3 παραμέτρους: 
//Το πλήθος των εναπομεινάντων καρτών στο ταμπλό (καρτών που δε βρίσκονται σε foundation), 
//το πλήθος των κινήσεων που απαιτούνται για να ελευθερωθεί μία κάρτα η οποία προορίζεται για foundation,
//την τιμή της κάρτας αυτής. 
//Η συγκεκριμένη συνάρτηση «τιμωρεί» σημαντικά το μεγάλο πλήθος εναπομεινάντων καρτών, και, σε μικρότερο βαθμό, των ελάχιστων κινήσεων, 
//αλλά επιβραβεύει σε μεγάλο βαθμό τις μετακινήσεις «μεγάλων» καρτών με ελάχιστες κινήσεις, 
//και σε μικρότερο βαθμό τις μετακινήσεις «μικρών» καρτών με αρκετές κινήσεις
int heuristic(board_state_t board_state)
{
	int score=0;
    int remaining_cards=0;
	int foundation_total=0;
	for (int i=0;i<FOUNDATIONS_NUM;i++)
        foundation_total+=board_state.foundation_card_sizes[i];

    remaining_cards=4*N - foundation_total;
    if (remaining_cards==0) return 0;

    int min_moves_suit;
    int min_moves_metric = min_moves_to_free_next_foundation_card(board_state,&min_moves_suit);
    int min_moves_suit_next_top_value = board_state.foundation_cards[min_moves_suit][board_state.foundation_card_sizes[min_moves_suit]].value+1;

    //Για παρτίδες με μη «θαμμένα» foundation-bases
    score = 15*remaining_cards
            + 5*min_moves_metric
            - 3*min_moves_metric*min_moves_suit_next_top_value;
	
	//Εναλλακτική ευρετική συνάρτηση
    //Για παρτίδες με κορυφαία φύλλα στις κορυφές των αρχικών στοιβών, στρατηγική η δημιουργία «καθαρών» στηλών για λύση
//	int empty_stacks;
//  int highest_value_card_opportunities = find_highest_value_card_opportunities(board_state, &empty_stacks);
//    score = 20*remaining_cards
//            + 5*min_moves_metric
//            - (min_moves_metric+1)*min_moves_suit_next_top_value
//            - remaining_cards*(empty_stacks+1)
//            - 4*highest_value_card_opportunities;


    return score;
}

// Υπολογίζει τις ελάχιστες κινήσεις που απαιτούνται για να ελευθερωθεί μία από τις τέσσερεις επόμενες κάρτες που προορίζονται για foundation.
// Σε περίπτωση ισοπαλίας, επιλέγεται η πρώτη κάρτα που βρέθηκε.
// Inputs:
//		board_state_t board_state: Η τρέχουσα κατάσταση του ταμπλό (ύστερα από τη νέα κίνηση)
//		int *min_moves_suit: Η φυλή της κάρτας για την οποία απαιτούνται οι ελάχιστες κινήσεις
// Output:
//		Κατ' αναφορά: *min_moves_suit: Η φυλή της κάρτας για την οποία απαιτούνται οι ελάχιστες κινήσεις
//		Επιστροφή: Το ελάχιστο πλήθος κινήσεων για να ελευθερωθεί μία κάρτα που προορίζεται για foundation
int min_moves_to_free_next_foundation_card(board_state_t board_state, int *min_moves_suit)
{
    int min_moves_to_free_next_foundation=100; //Ορισμός (υψηλής) αρχικής τιμής ελάχιστων κινήσεων

	//Ο εξωτερικός βρόχος ελέγχει για κάθε κορυφαίο φύλλο του foundation, πόσες κινήσεις απέχει το επόμενο φύλλο της ίδιας φυλής από το foundation
    for (int i=0;i<FOUNDATIONS_NUM;i++)
    {
        int foundation_i_suit = i; //Η φυλή του φύλλου αντιστοιχεί στις ορισμένες σταθερές με τιμές 0-3
        int foundation_i_next_top_value = board_state.foundation_card_sizes[foundation_i_suit]; //Η τιμή του επόμενου φύλλου της ίδιας φυλής που αναζητείται

		//Αν η τιμή που αναζητείται είναι ίση με Ν (όπου 0 έως Ν-1 οι τιμές των καρτών)
		//τότε το foundation της συγκεκριμένης φυλής έχει συμπληρωθεί, και εξετάζεται η επόμενη φυλή
		if (foundation_i_next_top_value==N)
            continue;

        int next_top_found=0; //Μεταβλητή flag για τον έλεγχο εύρεσης της κάρτας που αναζητείται
        int stack_depth; //Το «βάθος» της κάρτας στη στοίβα που ελέγχεται. Βάθος 0 υποδηλώνει πως η κάρτα βρίσκεται στην κορυφή της στοίβας

		//Έλεγχος ύπαρξης της κάρτας στα freecells
		//Ελέγχεται πρώτα αν υπάρχουν κάρτες στα freecells για να αποφευχθεί ο εσωτερικός βρόχος, σε περίπτωση έλλειψής τους.
		//Εάν η κάρτα εντοπιστεί στα freecells, τερματίζεται η εκτέλεση της συνάρτησης,
		//καθώς οι ελάχιστες κινήσεις που απαιτούνται για να ελευθερωθεί οποιαδήποτε κάρτα που προορίζεται για foundation είναι 0,
		//δηλαδή μπορεί στην επόμενη κίνηση να τοποθετηθεί σε foundation.
        if (board_state.freecell_size>0)
        {
            for (int j=0;j<FREECELLS_NUM;j++)
            {
                if (board_state.freecell_cards[j].suit==foundation_i_suit
                        && board_state.freecell_cards[j].value==foundation_i_next_top_value)
                    {
                        *min_moves_suit=foundation_i_suit;
                        return 0;
                    }
            }
        }

		//Έλεγχος ύπαρξης της κάρτας στις κύριες στοίβες
		//Εάν φτάσει στο σημείο αυτό η εκτέλεση, σημαίνει πως η κάρτα της συγκεκριμένης φυλής δεν εντοπίστηκε στα freecells
		//(καθώς θα είχε τερματιστεί η εκτέλεση της συνάρτησης στην περίπτωση αυτή).
		//Όταν βρεθεί η συγκεκριμένη κάρτα, συγκρίνεται το πλήθος των κινήσεων για να ελευθερωθεί η ίδια
		//με το μικρότερο πλήθος κινήσεων (min_moves_to_free_next_foundation) που έχει εντοπιστεί για άλλη κάρτα
		//(δεδομένου ότι δεν έχει τερματιστεί η εκτέλεση της συνάρτησης και έχει επιστρέψει 0 κινήσεις σε κάποια επανάληψη).
        for (int j=0;j<MAIN_STACKS_NUM;j++)
        {
            int current_stack_size = board_state.main_stack_sizes[j]; //Το μέγεθος της στοίβας που εξετάζεται
            for (int k=current_stack_size-1;k>=0;k--) //Η στοίβα εξετάζεται από την κορυφή της
            {
				//Η συνθήκη είναι αληθής αν η κάρτα που αναζητείται είναι η ίδια με την κάρτα [j][k]
                if (board_state.main_stack_cards[j][k].suit==foundation_i_suit
                    && board_state.main_stack_cards[j][k].value==foundation_i_next_top_value)
                {
                    next_top_found=1;
                    stack_depth=current_stack_size-k-1; //Όπου βάθος 0 υποδηλώνει ότι είναι στην κορυφή
                    break; //Τερματίζεται η εκτέλεση του for k
                }
            }
			//Εάν έχει εντοπιστεί η κάρτα, σταματάει ο έλεγχος στις κύριες στοίβες για τη συγκεκριμένη φυλή
            if (next_top_found)
                break; //Τερματίζεται η εκτέλεση του for j
        }

		//Στο σημείο αυτό, η κάρτα έχει εντοπιστεί στις κύριες στοίβες
		//Οι ακόλουθοι έλεγχοι εντοπίζουν, εν τέλει, την κάρτα που απαιτεί τις λιγότερες κινήσεις για να ελευθερωθεί
		//Η πρώτη συνθήκη, όπως και στην περίπτωση των freecells, είναι η ιδανική και τερματίζει άμεσα τη συνάρτηση,
		//καθώς η κάρτα που αναζητείται είναι στην κορυφή της στοίβας στην οποία βρίσκεται.
		//Η εναλλακτική συνθήκη συγκρίνει το πλήθος των ελάχιστων κινήσεων για να ελευθερωθεί η κάρτα
		//με όλες τις προηγούμενες μη μηδενικές τιμές, προκειμένου να εντοπιστεί η μικρότερη τιμή από αυτές.
        if (stack_depth==0)
        {
            min_moves_to_free_next_foundation=0;
            *min_moves_suit=foundation_i_suit;
            return 0;
        }
        else if (stack_depth>0 && stack_depth<min_moves_to_free_next_foundation)
        {
            min_moves_to_free_next_foundation=stack_depth;
            *min_moves_suit=foundation_i_suit;
        }
    }

	//Εάν η εκτέλεση φτάσει στο σημείο αυτό, σημαίνει πως καμία από τις τέσσερεις κάρτες που αναζητήθηκαν
	//δε βρισκόταν σε freecell ή κορυφή στοίβας, και συνεπώς επιστρέφεται μία μη μηδενική τιμή,
	//ίση με τις ελάχιστες κινήσεις που απαιτούνται για να ελευθερωθεί μία από τις τέσσερεις κάρτες αυτές.
    return min_moves_to_free_next_foundation;
}


// This function implements at the higest level the search algorithms.
// The various search algorithms differ only in the way the insert
// new nodes into the frontier, so most of the code is commmon for all algorithms.
// Inputs:
//		int method: Ο αλγόριθμος που επιλέχθηκε για την αναζήτηση της λύσης.
//		Καθολικές μεταβλητές: root, frontier_head and frontier_tail.
// Output:
//		NULL --> The problem cannot be solved
//		struct tree_node* --> A pointer to a search-tree leaf node that corresponds to a solution.
struct tree_node *search(int method)
{
	clock_t t;
	struct frontier_node *temp_frontier_node;
	struct tree_node *current_node;

	//ΕΝΤΟΛH DEBUG
	//int children_checked=0;

	while (frontier_head!=NULL)
	{
		t=clock();
		if (t-t1 > CLOCKS_PER_SEC*TIMEOUT)
		{
			printf("Timeout\n");
			return NULL;
		}

		// Extract the first node from the frontier
		current_node = frontier_head->n;

		if (is_solution(current_node))
			return current_node;

		// Delete the first node of the frontier
		temp_frontier_node=frontier_head;
		frontier_head = frontier_head->next;
		free(temp_frontier_node);
		if (frontier_head==NULL)
			frontier_tail=NULL;
		else
			frontier_head->previous=NULL;

		//ΕΝΤΟΛΕΣ DEBUG
		//children_checked++;
		//printf("# of children checked: %d", children_checked);

		// Find the children of the extracted node
		if (find_add_children(current_node, method)<0)
		{
			printf("Memory exhausted while creating new frontier node. Search is terminated...\n");
			return NULL;
		}
	}

    return NULL;
}

//Εξετάζεται η δυνατότητα ικανοποίησης του προβλήματος
//Ελέγχεται αν τα κορυφαία φύλλα και των 4 foundations είναι Ρήγες (για Ν=13, ειδάλλως αν είναι τιμής Ν-1)
// Inputs:
//		struct tree_node *node: Ο κόμβος που εξετάζεται ως πιθανή λύση
// Output:
//		0 --> Ο κόμβος δεν αποτελεί λύση
//		1 --> Ο κόμβος αποτελεί λύση
int is_solution(struct tree_node *node)
{
    int i;
	board_state_t current_board_state=node->board_state;

    for(i=0;i<FOUNDATIONS_NUM;i++)
        if (current_board_state.foundation_cards[i][N-1].value!=N-1)
			return 0;

	//Η έξοδος από το βρόχο σημαίνει πως όλα τα φύλλα των foundations είναι τιμής Ν-1, και συνεπώς η παρτίδα έχει λυθεί
	return 1;
}

// Η συνάρτηση εντοπίζει όλες τις πιθανές κινήσεις που μπορούν να προκύψουν από την κατάσταση του ταμπλό
// Τα πιθανά είδη (μη συμμετρικών) κινήσεων είναι 7, ανάλογα με την προέλευση και τον προορισμό των κινήσεων:
//	1) Προέλευση: Freecell
//		1α) Προορισμός: Στοίβα (άδεια ή μη)
//		1β) Προορισμός: Foundation
//	2) Προέλευση: Foundation
//		2α) Προορισμός: Στοίβα (άδεια ή μη)
//		2β) Προορισμός: Freecell
//	3) Προέλευση: Κύρια στοίβα
//		3α) Προορισμός: Στοίβα (άδεια ή μη)
//		3β) Προορισμός: Freecell
//		3γ) Προορισμός: Foundation
//
// Για κάθε είδος προέλευσης, λαμβάνεται η κορυφαία κάρτα της του τομέα της προέλευσης (της κάθε στοίβας/foundation ή η κάθε κάρτα των freecell),
// και εξετάζεται το αν υπάρχει διαθέσιμη κίνηση μέσω των συναρτήσεων check_add_card.
// Inputs:
//		struct tree_node *current_node	: A leaf-node of the search tree.
//		int method: Ο αλγόριθμος που επιλέχθηκε για την αναζήτηση της λύσης.
// Output:
//		1 --> Ελέγχθηκαν όλες οι πιθανές κινήσεις επιτυχώς, χωρίς θέματα μνήμης
//		-1 --> Κρίσιμο σφάλμα μνήμης κατά τη δημιουργία παιδιού ή κατά την προσθήκη παιδιού στο μέτωπο αναζήτησης (μέσω συναρτήσεων check_add_card)
int find_add_children(struct tree_node *current_node, int method)
{
//	ΕΝΤΟΛΕΣ DEBUG
// 	printf("----------CURRENT NODE STATE---------------------------\n");
//	printf("Node evaluation: %d\n", current_node->f);
//	printf("Node depth: %d\n", current_node->g);
//	display_board_state(current_node);

	//1) Προέλευση: Freecell
	//Εξετάζεται το πλήθος των κατειλλημένων freecells, για να αποφευχθεί ο εσωτερικός βρόχος for σε περίπτωση απουσίας καρτών
	//Πιθανοί προορισμοί: Στοίβα (άδεια ή μη), Foundation
	if (current_node->board_state.freecell_size>0)
		for(int i=0;i<FREECELLS_NUM;i++)
		{
			card_t *curr_i_freecell_card = &current_node->board_state.freecell_cards[i]; //Κάρτα θέσης i στα freecells
			if (curr_i_freecell_card->value!=-1) //Τιμή -1 υποδηλώνει κενό freecell
			{
				if (   check_add_card_to_stack      (current_node, curr_i_freecell_card, i, FREECELL, method) == -1
                    || check_add_card_to_foundation (current_node, curr_i_freecell_card, i, FREECELL, method) == -1) return -1;
			}
		}

	//2) Προέλευση: Foundation
	//Πιθανοί προορισμοί: Στοίβα (άδεια ή μη), Freecell
	for(int i=0;i<FOUNDATIONS_NUM;i++)
	{
		int curr_i_foundation_size=current_node->board_state.foundation_card_sizes[i]; //Μέγεθος της στοίβας φυλής i
		if (curr_i_foundation_size>0) //Αν το μέγεθος της στοίβας φυλής i είναι 0, δεν υπάρχει κάρτα προς μετακίνηση
		{
			card_t *curr_i_foundation_top_card = &current_node->board_state.foundation_cards[i][curr_i_foundation_size-1]; //Κορυφαία κάρτα στο foundation φυλής i
			if (   check_add_card_to_stack	    (current_node, curr_i_foundation_top_card, i, FOUNDATION, method) == -1
                || check_add_card_to_freecell	(current_node, curr_i_foundation_top_card, i, FOUNDATION, method) == -1) return -1;
		}
	}

	//3) Προέλευση: Κύρια στοίβα
	//Πιθανοί προορισμοί: Στοίβα (άδεια ή μη), Freecell, Foundation
	for(int i=0;i<MAIN_STACKS_NUM;i++)
	{
		int curr_i_stack_size=current_node->board_state.main_stack_sizes[i]; //Μέγεθος της στοίβας i
		if (curr_i_stack_size>0) //Αν το μέγεθος της στοίβας i είναι 0, δεν υπάρχει κάρτα προς μετακίνηση, δηλαδή η στοίβα είναι άδεια
		{
			card_t *curr_i_stack_top_card = &current_node->board_state.main_stack_cards[i][curr_i_stack_size-1]; //Κορυφαία κάρτα στην κύρια στοίβα
			if(	   check_add_card_to_stack      (current_node, curr_i_stack_top_card, i, STACK, method) == -1
                || check_add_card_to_freecell	(current_node, curr_i_stack_top_card, i, STACK, method) == -1
                || check_add_card_to_foundation (current_node, curr_i_stack_top_card, i, STACK, method) == -1) return -1;

		}
	}

	return 1;
}

//Συνάρτηση ελέγχου μετακίνησης κάρτας σε στοίβα (άδεια ή μη)
// Inputs:
//		struct tree_node *current_node	: A leaf-node of the search tree.
//		card_t *curr_i_card: Η κάρτα η οποία εξετάζεται προς μετακίνηση.
//		int i: Ο αριθμοδείκτης της: Θέσης της κάρτας στον πίνακα των freecell / Κύριας στοίβας στην οποία ανήκει η κάρτα / Φυλής στην οποία ανήκει η κάρτα στα foundations
//		int move_from: Η προέλευση της κάρτας (STACK, FREECELL, FOUNDATION)
//		int method: Ο αλγόριθμος που επιλέχθηκε για την αναζήτηση της λύσης.
// Output:
//		0 --> Δημιουργήθηκαν όλα τα πιθανά (μη συμμετρικά) παιδιά επιτυχώς, χωρίς θέματα μνήμης
//		-1 --> Κρίσιμο σφάλμα μνήμης κατά τη δημιουργία παιδιού ή κατά την προσθήκη παιδιού στο μέτωπο αναζήτησης (μέσω συνάρτησης add_new_child)
int check_add_card_to_stack(struct tree_node *current_node, card_t *curr_i_card, int i, int move_from, int method)
{
    int move_to_empty_stack_visited=0; //Μεταβλητή flag για την αποφυγή δημιουργίας οριζόντιων συμμετρικών παιδιών αν η μετακίνηση είναι τύπου NEW_STACK,
										//καθώς τα παιδιά που δημιουργούνται είναι ισοδύναμα κατά τη μετακίνηση σε οποιαδήποτε άδεια στήλη

	//Ο εξωτερικός επαναληπτικός βρόχος εξετάζει τις κορυφές των κύριων στοιβών για πιθανά «ζευγάρια»
	for (int j=0;j<MAIN_STACKS_NUM;j++)
	{
		int stack_size=current_node->board_state.main_stack_sizes[j]; //Μέγεθος της στοίβας j
		if (move_from==STACK && i==j) //Αν η curr_i_card προέρχεται από στήλη, δεν εξετάζεται η μετακίνηση προς την στήλη της
			continue;
		else if (stack_size>0) //Μη άδεια στήλη, εξετάζεται η τοποθέτηση στην κορυφή της, κίνηση τύπου STACK
		{
			card_t *stack_top_card = &current_node->board_state.main_stack_cards[j][stack_size-1]; //Κορυφαία κάρτα στην στοίβα j

			//Η συνθήκη εξετάζει αν είναι δυνατή η μετακίνηση της curr_i_card πάνω στην stack_top_card
			//Εξετάζονται οι συνθήκες: Το χρώμα της curr_i_card να είναι διαφορετικό της stack_top_card
			//ΚΑΙ η τιμή της curr_i_card είναι κατά 1 μικρότερη της stack_top_card
			if (curr_i_card->color!=stack_top_card->color && curr_i_card->value==stack_top_card->value-1)
			{
				if (add_new_child(current_node, i, j, STACK, move_from, curr_i_card, stack_top_card, method)==-1)
					return -1;
			}
		}
		else if (stack_size==0 && !move_to_empty_stack_visited) //Τοποθέτηση σε κενή στήλη, κίνηση τύπου NEW_STACK
		{
			if (add_new_child(current_node, i, j, NEW_STACK, move_from, curr_i_card, NULL, method)==-1)
                return -1;
            move_to_empty_stack_visited=1;
		}
	}

	return 0;
}

//Συνάρτηση ελέγχου μετακίνησης κάρτας σε foundation
// Inputs:
//		struct tree_node *current_node	: A leaf-node of the search tree.
//		card_t *curr_i_card: Η κάρτα η οποία εξετάζεται προς μετακίνηση.
//		int i: Ο αριθμοδείκτης της: Θέσης της κάρτας στον πίνακα των freecell / Κύριας στοίβας στην οποία ανήκει η κάρτα
//		int move_from: Η προέλευση της κάρτας (STACK, FREECELL, FOUNDATION)
//		int method: Ο αλγόριθμος που επιλέχθηκε για την αναζήτηση της λύσης.
// Output:
//		0 --> Δημιουργήθηκαν όλα τα πιθανά (μη συμμετρικά) παιδιά επιτυχώς, χωρίς θέματα μνήμης
//		-1 --> Κρίσιμο σφάλμα μνήμης κατά τη δημιουργία παιδιού ή κατά την προσθήκη παιδιού στο μέτωπο αναζήτησης (μέσω συνάρτησης add_new_child)
int check_add_card_to_foundation(struct tree_node *current_node, card_t *curr_i_card, int i, int move_from, int method)
{
	int foundation_same_suit_top_value = current_node->board_state.foundation_card_sizes[curr_i_card->suit] - 1; //Η κορυφαία τιμή στο foundation στο οποίο ανήκει η κάρτα

	//Ο έλεγχος είναι επιτυχής αν η τιμή της curr_i_card είναι κατά 1 μεγαλύτερη αυτής της τρέχουσας κορυφαίας κάρτας foundation_same_suit_top_value
	if (curr_i_card->value == foundation_same_suit_top_value + 1)
	{
		if (add_new_child(current_node, i, curr_i_card->suit, FOUNDATION, move_from, curr_i_card, NULL, method) == -1)
            return -1;
	}
	return 0;
}

//Συνάρτηση ελέγχου μετακίνησης κάρτας σε freecell
// Inputs:
//		struct tree_node *current_node	: A leaf-node of the search tree.
//		card_t *curr_i_card: Η κάρτα η οποία εξετάζεται προς μετακίνηση.
//		int i: Ο αριθμοδείκτης της: Φυλής στην οποία ανήκει η κάρτα στα foundations / Κύριας στοίβας στην οποία ανήκει η κάρτα
//		int move_from: Η προέλευση της κάρτας (STACK, FREECELL, FOUNDATION)
//		int method: Ο αλγόριθμος που επιλέχθηκε για την αναζήτηση της λύσης.
// Output:
//		0 --> Δημιουργήθηκαν όλα τα πιθανά (μη συμμετρικά) παιδιά επιτυχώς, χωρίς θέματα μνήμης
//		-1 --> Κρίσιμο σφάλμα μνήμης κατά τη δημιουργία παιδιού ή κατά την προσθήκη παιδιού στο μέτωπο αναζήτησης (μέσω συνάρτησης add_new_child)
int check_add_card_to_freecell(struct tree_node *current_node, card_t *curr_i_card, int i, int move_from, int method)
{
	if (current_node->board_state.freecell_size!=FREECELLS_NUM) //Εξετάζεται αν είναι κατειλημμένα όλα τα freecells
	{
		//Στο σημείο αυτό είναι βέβαιο πως υπάρχει άδειο freecell,
		//και η μεταβλητή θα αποθηκεύσει τον αριθμοδείκτη της πρώτης άδειας θέσης που θα εντοπιστεί
		int empty_freecell_index = -1;
		for (int j=0;j<FREECELLS_NUM;j++)
        {
            if (current_node->board_state.freecell_cards[j].value==-1) //Το freecell θεωρείται άδειο αν η τιμή του είναι -1
			{
				empty_freecell_index=j;
				break; //Η τοποθέτηση ενός φύλλου σε ένα άδειο freecell είναι ισοδύναμη με την τοποθέτησή του σε ένα διαφορετικό άδειο freecell.
						//Συνεπώς, για την αποφυγή δημιουργίας οριζόντιων συμμετρικών παιδιών, αρκεί να βρεθεί το πρώτο άδειο freecell.
			}
        }

		if (empty_freecell_index>-1)
			if (add_new_child(current_node, i, empty_freecell_index, FREECELL, move_from, curr_i_card, NULL, method)==-1)
                return -1;
	}

	return 0;
}

// Η συνάρτηση επιχειρεί τη δημιουργία ενός νέου κόμβου-παιδιού
// Inputs:
//		struct tree_node *parent: Ο γονέας του νέου παιδιού
//		int source_i: Για την προέλευση της κάρτας που μετακινείται, ο αριθμοδείκτης της: Θέσης της κάρτας στον πίνακα των freecell / Κύριας στοίβας στην οποία ανήκει η κάρτα / Φυλής στην οποία ανήκει η κάρτα στα foundations
//		int dest_i: Για τον προορισμό της κάρτας που μετακινείται, ο αριθμοδείκτης της: Άδειας θέσης του freecell / Nέας της στοίβας / Φυλής στην οποία ανήκει η κάρτα στα foundations
//		int move_type: Ο τύπος κίνησης / προορισμός της κάρτας (STACK, FREECELL, FOUNDATION, NEW_STACK)
//		int move_from: Η προέλευση της κάρτας (STACK, FREECELL, FOUNDATION)
//		card_t* card_moved: Η κάρτα που μετακινείται
//		card_t* card_below: Για κίνηση τύπου STACK, η κάρτα πάνω από την οποία θα τοποθετηθεί, ειδάλλως NULL
//		int method: Ο αλγόριθμος που επιλέχθηκε για την αναζήτηση της λύσης.
// Output:
//		0 --> Το παιδί δημιουργήθηκε επιτυχώς και είναι μοναδικό στο κλαδί του, χωρίς θέματα μνήμης
//		1 --> Το παιδί δημιουργήθηκε αλλά προϋπήρχε στο κλαδί πρόγονος με την ίδια ακριβώς κατάσταση / ταμπλό
//		-1 --> Κρίσιμο σφάλμα μνήμης κατά τη δημιουργία παιδιού ή κατά την προσθήκη παιδιού στο μέτωπο αναζήτησης
int add_new_child(struct tree_node *parent, int source_i, int dest_i, int move_type, int move_from, card_t* card_moved, card_t* card_below, int method)
{
	struct tree_node *child=(struct tree_node*) malloc(sizeof(struct tree_node));
	if (child==NULL) return -1;

	//Καταγραφή των στοιχείων του κόμβου-παιδιού
	child->parent=parent;
	child->g=parent->g + 1;
	child->card_movement.move_type=move_type;
	child->card_movement.move_from=move_from;
	child->card_movement.card_moved=card_moved;
	child->card_movement.card_below=card_below;

    // Αντιγραφή του board_state
    copy_board_state(child,parent->board_state);

	//Προσαρμογή του board_state για τη νέα κίνηση
	//7 περιπτώσεις από 3 προορισμούς
	//1) Προορισμός STACK/NEW_STACK:
	//	1a) Από διαφορετικό STACK, 1b) Από FREECELL, 1c) Από FOUNDATION
	//2) Προορισμός FREECELL:
	//	2a) Από STACK, 2b) Από FOUNDATION
	//3) Προορισμός FOUNDATION:
	//	3a) Από STACK, 3b) Από FREECELL

    int dest_i_new_top; //Η νέα κορυφή της στοίβας (για κινήσεις STACK, NEW_STACK, FOUNDATION)

	//Περίπτωση 1
	if (child->card_movement.move_type==STACK || child->card_movement.move_type==NEW_STACK)
	{
		dest_i_new_top = child->board_state.main_stack_sizes[dest_i];
		move_card(child, source_i, dest_i, dest_i_new_top);
	}
	//Περίπτωση 2
	else if (child->card_movement.move_type==FREECELL)
	{
        move_card(child, source_i, dest_i, dest_i);
    }
	//Περίπτωση 3
	else if (child->card_movement.move_type==FOUNDATION)
	{
		int card_moved_suit = child->card_movement.card_moved->suit;
		//Το μέγεθος του foundation πριν τη μετακίνηση είναι size, όπου size=Τιμή υψηλότερου φύλλου+1.
		//Συνεπώς, ο προορισμός του εξεταζόμενου φύλλου στη θέση με αριθμοδείκτη size=dest_i_new_top, ο οποίος θα αυξηθεί κατά 1 μετά την τοποθέτηση
		dest_i_new_top = child->board_state.foundation_card_sizes[card_moved_suit];
		move_card(child, source_i, dest_i, dest_i_new_top);
	}

    //Check for loops
    if (!check_with_parents(child))
    {
        // In case of loop detection, the child is deleted
        free(child);
        return 1;
    }
    // Computing the heuristic value
    calc_child_node_value(child,method);

	int err=0;
	if (method==DEPTH)
		err=add_frontier_front(child);
	else if (method==BREADTH)
		err=add_frontier_back(child);
	else if (method==BEST || method==A_STAR)
		err=add_frontier_in_order(child);

	if (err<0)
		return -1;

    return 0;
}

// Η συνάρτηση αντιγράφει την κατάσταση του ταμπλό του γονέα στο παιδί πριν πραγματοποιηθεί η μετακίνηση
// Inputs:
//		struct tree_node *child: Ο κόμβος-παιδί
//		board_state_t parent_board_state: Η κατάσταση του ταμπλό του γονέα
// Output:
//		Κατ' αναφορά: Το παιδί, με την κατάσταση του ταμπλό του γονέα
void copy_board_state(struct tree_node *child, board_state_t parent_board_state)
{
    copy_main_stack(child, parent_board_state);
    copy_foundations(child, parent_board_state);
    copy_freecells(child, parent_board_state);
}

// Η συνάρτηση αντιγράφει την κατάσταση των κύριων στοιβών από το ταμπλό του γονέα στο παιδί πριν πραγματοποιηθεί η μετακίνηση
// Inputs:
//		struct tree_node *child: Ο κόμβος-παιδί
//		board_state_t parent_board_state: Η κατάσταση του ταμπλό του γονέα
// Output:
//		Κατ' αναφορά: Το παιδί, με την κατάσταση των κύριων στοιβών από το ταμπλό του γονέα
void copy_main_stack(struct tree_node *child, board_state_t parent_board_state)
{
    initialize_main_stack_state(&child->board_state);
    for (int i=0;i<MAIN_STACKS_NUM;i++)
    {
        int stack_size = child->board_state.main_stack_sizes[i]=parent_board_state.main_stack_sizes[i];
        for (int j=0;j<stack_size;j++)
            child->board_state.main_stack_cards[i][j] = parent_board_state.main_stack_cards[i][j];
    }
}

// Η συνάρτηση αντιγράφει την κατάσταση των foundations από το ταμπλό του γονέα στο παιδί πριν πραγματοποιηθεί η μετακίνηση
// Inputs:
//		struct tree_node *child: Ο κόμβος-παιδί
//		board_state_t parent_board_state: Η κατάσταση του ταμπλό του γονέα
// Output:
//		Κατ' αναφορά: Το παιδί, με την κατάσταση των foundations από το ταμπλό του γονέα
void copy_foundations(struct tree_node *child, board_state_t parent_board_state)
{
    initialize_foundations_state(&child->board_state);
    for (int i=0;i<FOUNDATIONS_NUM;i++)
    {
        int stack_size = child->board_state.foundation_card_sizes[i]=parent_board_state.foundation_card_sizes[i];
        for (int j=0;j<stack_size;j++)
            child->board_state.foundation_cards[i][j] = parent_board_state.foundation_cards[i][j];
    }
}

// Η συνάρτηση αντιγράφει την κατάσταση των freecells από το ταμπλό του γονέα στο παιδί πριν πραγματοποιηθεί η μετακίνηση
// Inputs:
//		struct tree_node *child: Ο κόμβος-παιδί
//		board_state_t parent_board_state: Η κατάσταση του ταμπλό του γονέα
// Output:
//		Κατ' αναφορά: Το παιδί, με την κατάσταση των freecells από το ταμπλό του γονέα
void copy_freecells(struct tree_node *child, board_state_t parent_board_state)
{
    initialize_freecell_state(&child->board_state);
    child->board_state.freecell_size=parent_board_state.freecell_size;
    for (int i=0;i<FREECELLS_NUM;i++)
        child->board_state.freecell_cards[i] = parent_board_state.freecell_cards[i];
}

// Η συνάρτηση επιχειρεί τη δημιουργία ενός νέου κόμβου-παιδιού
// Inputs:
//		struct tree_node *child: Το νέο παιδί
//		int source_i: Για την προέλευση της κάρτας που μετακινείται, ο αριθμοδείκτης της: Θέσης της κάρτας στον πίνακα των freecell / Κύριας στοίβας στην οποία ανήκει η κάρτα / Φυλής στην οποία ανήκει η κάρτα στα foundations
//		int dest_i: Για τον προορισμό της κάρτας που μετακινείται, ο αριθμοδείκτης της: Άδειας θέσης του freecell / Nέας της στοίβας / Φυλής στην οποία ανήκει η κάρτα στα foundations
//		int dest_i_new_top: Για τον προορισμό της κάρτας που μετακινείται, ο αριθμοδείκτης της θέσης της στη νέα της στοίβα / στο foundation (για κινήσεις τύπου STACK, FOUNDATION, NEW_STACK)
//		int move_type: Ο τύπος κίνησης / προορισμός της κάρτας (STACK, FREECELL, FOUNDATION, NEW_STACK)

// Output:
//		Κατ' αναφορά: Το παιδί, με αλλαγές στην κατάσταση του ταμπλό του
void move_card(struct tree_node *child, int source_i, int dest_i, int dest_i_new_top)
{
	//Αρχικά, προστίθεται η card_moved στην επιλεγμένη κορυφή-προορισμό
	//Στη συνέχεια, το μέγεθος της στοίβας αυξάνεται κατά 1 / το μέγεθος των κατειλλημένων freecells αυξάνεται κατά 1
	if (child->card_movement.move_type==STACK || child->card_movement.move_type==NEW_STACK)
    {

        child->board_state.main_stack_cards[dest_i][dest_i_new_top] = *child->card_movement.card_moved;
        child->board_state.main_stack_sizes[dest_i]++;
    }
	else if (child->card_movement.move_type==FREECELL)
    {
        child->board_state.freecell_cards[dest_i_new_top] = *child->card_movement.card_moved;
        child->board_state.freecell_size++;
    }

	else if (child->card_movement.move_type==FOUNDATION)
    {
        child->board_state.foundation_cards[dest_i][dest_i_new_top] = *child->card_movement.card_moved;
        child->board_state.foundation_card_sizes[dest_i]++;
    }


	//Έπειτα, στην αρχική θέση-προέλευση της κάρτας, η τιμή της ορίζεται ως -1,
	//και το μέγεθος της στοίβας αυτής μειώνεται κατά 1 / το μέγεθος των κατειλλημένων freecells μειώνεται κατά 1
	if (child->card_movement.move_from==STACK)
    {
        child->board_state.main_stack_cards[source_i][child->board_state.main_stack_sizes[source_i]-1].value=-1;
        child->board_state.main_stack_sizes[source_i]--;
    }
	else if (child->card_movement.move_from==FREECELL)
    {
        child->board_state.freecell_cards[source_i].value=-1;
        child->board_state.freecell_size--;
    }
	else if (child->card_movement.move_from==FOUNDATION)
    {
        child->board_state.foundation_cards[source_i][child->board_state.foundation_card_sizes[source_i]-1].value=-1;
        child->board_state.foundation_card_sizes[source_i]--;
    }
}

// This function checks whether a node in the search tree
// holds exactly the same puzzle with at least one of its
// predecessors. This function is used when creating the childs
// of an existing search tree node, in order to check for each one of the childs
// whether this appears in the path from the root to its parent.
// This is a moderate way to detect loops in the search.
// Inputs:
//		struct tree_node *new_node	: A search tree node (usually a new one)
// Output:
//		1 --> No coincidence with any predecessor
//		0 --> Loop detection
int check_with_parents(struct tree_node *new_node)
{
	struct tree_node *parent=new_node->parent;
	while (parent!=NULL)
	{
		if (identical_board_states(new_node, parent))
			return 0;
		parent=parent->parent;
	}
	return 1;
}

// Η συνάρτηση ελέγχει την ισότητα των κατάστασεων δύο κόμβων στους 3 τομείς του ταμπλό.
// Inputs:
//		struct tree_node *new_node: Ο κόμβος-παιδί που εξετάζεται
//		struct tree_node *ascendant_node: Ο κόμβος-πρόγονος με τον οποίο συγκρίνεται το παιδί
// Output:
//		1 --> Οι καταστάσεις είναι πανομοιότυπες
//		0 --> Οι καταστάσεις είναι διαφορετικές
int identical_board_states(struct tree_node *new_node, struct tree_node *ascendant_node)
{
	return (equal_freecells(new_node, ascendant_node)
			&& equal_foundations(new_node, ascendant_node)
			&& equal_main_stacks(new_node, ascendant_node));

}

// Η συνάρτηση ελέγχει την ισότητα των freecells των κατάστασεων δύο κόμβων.
// Μελετάται το περιεχόμενο των freecells κι όχι η σειρά τους
// Inputs:
//		struct tree_node *new_node: Ο κόμβος-παιδί που εξετάζεται
//		struct tree_node *ascendant_node: Ο κόμβος-πρόγονος με τον οποίο συγκρίνεται το παιδί
// Output:
//		1 --> Τα freecells είναι πανομοιότυπα στα περιεχόμενά τους
//		0 --> Τα freecells είναι διαφορετικά στα περιεχόμενά τους
int equal_freecells(struct tree_node *new_node, struct tree_node *ascendant_node)
{
	int new_node_size = new_node->board_state.freecell_size;			 //Κατειλημμένα freecells του παιδιού
	int ascendant_node_size = ascendant_node->board_state.freecell_size; //Κατειλημμένα freecells του προγόνου

	//Αρχικά εξετάζεται το μέγεθος των δύο κόμβων
	//Αν και οι δύο έχουν άδεια freecells, θεωρούνται πανομοιότυποι στον τομέα αυτό
	if (new_node_size==0 && ascendant_node_size==0)
        return 1;

	//Αν έχουν διαφορετικά μεγέθη, προφανώς είναι διαφορετικές οι κατάστασεις των freecell τους
	if (new_node_size != ascendant_node_size)
		return 0;

	//Στο σημείο αυτό, έχουν αποκλειστεί τα ενδεχόμενα των άδειων freecells και των διαφορετικών μεγεθών.
	//Συνεπώς, οι δύο κόμβοι έχουν το ίδιο πλήθος κατειλλημένων φύλλων στα freecell τους.
	//Στη συνέχεια, εξετάζονται τα φύλλα των freecell των δύο κόμβων για να επιβεβαιωθεί ή να διαψευσθεί αντιστοιχία 1 προς 1,
	//δηλαδή, αν για κάθε φύλλο του παιδιού υπάρχει το ίδιο φύλλο στα freecells του προγόνου
	for (int i=0;i<FREECELLS_NUM;i++)
	{
		card_t curr_card = new_node->board_state.freecell_cards[i]; //Κάρτα του παιδιού
		if (curr_card.value!=-1) //Ο έλεγχος ξεκινά εφόσον το freecell του παιδιού που εξετάζεται δεν είναι άδειο
        {
            int identical_found=0; //Μεταβλητή flag για τον εντοπισμό ίδιου φύλλου στα freecells του προγόνου
            for (int j=0;j<FREECELLS_NUM;j++)
            {
                card_t ascendant_card = ascendant_node->board_state.freecell_cards[j]; //Κάρτα του προγόνου

				//Η συνθήκη είναι αληθής αν οι τιμές και οι φυλές των δύο καρτών είναι πανομοιότυπες
				//Στην περίπτωση αυτή, ο εσωτερικός επαναληπτικός βρόχος σταματάει την εκτέλεσή του, και εξετάζεται η επόμενη κάρτα του παιδιού
                if (curr_card.value==ascendant_card.value && curr_card.suit==ascendant_card.suit)
                {
                    identical_found=1;
                    break;
                }
            }

			//Η συνθήκη είναι αληθής αν και μόνο αν δεν αλλάξει η τιμή της μεταβλητής flag στον εσωτερικό επαναληπτικό βρόχο,
			//δηλαδή αν για την κάρτα του παιδιού δε βρεθεί πανομοιότυπη κάρτα στα freecells του προγόνου,
			//και συνεπώς οι καταστάσεις των freecell τους, και, κατ' επέκταση, των ταμπλό τους δεν είναι πανομοιότυπες
            if (!identical_found)
				return 0;
        }
	}

	//Εάν βγει από τον εξωτερικό επαναληπτικό βρόχο η εκτέλεση, σημαίνει πως υπάρχει 1-1 αντιστοιχία
	return 1;
}

// Η συνάρτηση ελέγχει την ισότητα των foundations των κατάστασεων δύο κόμβων.
// Inputs:
//		struct tree_node *new_node: Ο κόμβος-παιδί που εξετάζεται
//		struct tree_node *ascendant_node: Ο κόμβος-πρόγονος με τον οποίο συγκρίνεται το παιδί
// Output:
//		1 --> Τα foundations είναι πανομοιότυπα στα περιεχόμενά τους
//		0 --> Τα foundations είναι διαφορετικά στα περιεχόμενά τους
int equal_foundations(struct tree_node *new_node, struct tree_node *ascendant_node)
{
	//Με δεδομένο ότι στο κάθε foundation αντιστοιχεί μία συγκεκριμένη φυλή (με ρητά ορισμένο αριθμοδείκτη 0-3),
	//αρκεί να εξεταστούν τα μεγέθη των foundation με τον ίδιο αριθμοδείκτη στους δύο κόμβους

	for (int i=0;i<FOUNDATIONS_NUM;i++)
		if (new_node->board_state.foundation_card_sizes[i] != ascendant_node->board_state.foundation_card_sizes[i])
			return 0;

	//Εάν βγει από τον επαναληπτικό βρόχο η εκτέλεση, σημαίνει πως υπάρχει 1-1 αντιστοιχία
	return 1;
}

// Η συνάρτηση ελέγχει την ισότητα των κύριως στοιβών των κατάστασεων δύο κόμβων.
// Μελετάται το περιεχόμενο των κυρίων στοιβών ΚΑΙ η σειρά των περιεχομένων τους
// Inputs:
//		struct tree_node *new_node: Ο κόμβος-παιδί που εξετάζεται
//		struct tree_node *ascendant_node: Ο κόμβος-πρόγονος με τον οποίο συγκρίνεται το παιδί
// Output:
//		1 --> Οι κύριες στοίβες είναι πανομοιότυπες στα περιεχόμενά τους
//		0 --> Οι κύριες στοίβες είναι διαφορετικές στα περιεχόμενά τους
int equal_main_stacks(struct tree_node *new_node, struct tree_node *ascendant_node)
{
	board_state_t new_node_board = new_node->board_state; 		//Κατάσταση ταμπλό του παιδιού
	board_state_t ascendant_board = ascendant_node->board_state;//Κατάσταση ταμπλό του προγόνου

	int identical_stacks=0; //Μεταβλητή flag για τον εντοπισμό πανομοιότυπων στοιβών
	int new_node_empty_main_stacks=0; //Μεταβλητή για το πλήθος των άδειων στηλών του παιδιού
	int ascendant_node_empty_main_stacks=0; //Μεταβλητή για το πλήθος των άδειων στηλών του προγόνου

	//Καταμέτρηση των άδειων στηλών των δύο κόμβων
	for (int i=0;i<MAIN_STACKS_NUM;i++)
    {
        if (new_node_board.main_stack_sizes[i]==0)
            new_node_empty_main_stacks++;

        if (ascendant_board.main_stack_sizes[i]==0)
            ascendant_node_empty_main_stacks++;
    }

	//Αν οι δύο κόμβοι δεν έχουν το ίδιο πλήθος άδειων στηλών, οι κατάστάσεις τους είναι διαφορετικές
    if (new_node_empty_main_stacks!=ascendant_node_empty_main_stacks)
        return 0;

	//Εφόσον συνεχιστεί η εκτέλεση, το πλήθος των άδειων στηλών των κόμβων είναι ίσο,
	//και συνεπώς επιβεβαιώθηκε η ομοιότητά τους σε ένα ποσοστό, το οποίο δε χρειάζεται να επανεξεταστεί κατά τον έλεγχο των μη άδειων στηλών
	//Η τιμή της μεταβλητής ανανεώνεται το πολύ μία φορά ανά εκτέλεση του δεύτερου βρόχου (αναλύεται παρακάτω)
    identical_stacks+=new_node_empty_main_stacks;

	//Μεταβλητή-μετρητής για το πλήθος των πανομοιότυπων στηλών ανάμεσα στους δύο κόμβους
	//Αρχικά, το πλήθος είναι ίσο με τις άδειες στήλες που καταγράφηκαν προηγουμένως
	//Πριν το τέλος της εκτέλεσης της κάθε επανάληψης του πρώτου βρόχου (αναλύεται παρακάτω), συγκρίνεται με το καταγεγραμμένο πλήθος
	//των πανομοιότυπων στηλών (identical_stacks), και, αν είναι ίσο, οι καταστάσεις δεν είναι πανομοιότυπες,
	//ειδάλλως λαμβάνει τη νέα τιμή της identical_stacks.
    int current_identical_stacks=identical_stacks;

	//Εκτελούνται συνολικά 3 επαναληπτικοί βρόχοι ταυτόχρονα:
	//Ο πρώτος βρόχος εξετάζει την κάθε μη άδεια στήλη του παιδιού, έστω i
	//Ο δεύτερος βρόχος, για κάθε στήλη i, εξετάζει την ομοιότητά της με κάθε στήλη του προγόνου, έστω j
	//Ο τρίτος βρόχος, για κάθε στήλη i και j, εξετάζει την 1-1 ομοιότητά τους
	for (int i=0;i<MAIN_STACKS_NUM;i++) //Βρόχος πρώτος
	{
	    if (new_node_board.main_stack_sizes[i]!=0) //Καθώς οι άδειες στήλες καταμετρήθηκαν προηγουμένως, δεν επαναλαμβάνεται ο έλεγχός τους
        {
            for (int j=0;j<MAIN_STACKS_NUM;j++) //Βρόχος δεύτερος
            {
                //Αρχικά εξετάζονται τα μεγέθη των στηλών i και j
				//Αν τα μεγέθη τους είναι διαφορετικά, προφανώς και δεν υφίσταται λόγος εξέτασής τους
                if (new_node_board.main_stack_sizes[i]==ascendant_board.main_stack_sizes[j])
                {
					int is_identical_stack=1; //Μεταβλητή flag για τον έλεγχο ομοιότητας των δύο στηλών

                    for (int k=0;k<new_node_board.main_stack_sizes[i];k++) //Βρόχος τρίτος
                    {
						//Η συνθήκη είναι αληθής αν οι δύο κάρτες στις στήλες i και j στη θέση k είναι διαφορετικές
						//Στην περίπτωση αυτή, οι δύο στήλες δεν είναι πανομοιότυπες, και συνεπώς δεν εξετάζονται περαιτέρω
                        if (new_node_board.main_stack_cards[i][k].value!=ascendant_board.main_stack_cards[j][k].value
                        || new_node_board.main_stack_cards[i][k].suit!=ascendant_board.main_stack_cards[j][k].suit)
                        {
                            is_identical_stack=0;
                            break;
                        }
                    }

					//Αν η τιμή της μεταβλητής flag δεν αλλάξει στον τρίτο επαναληπτικό βρόχο,
					//σημαίνει πως οι στήλες i και j είναι πανομοιότυπες στα περιεχόμενά τους, και το πλήθος των όμοιων στηλών αυξάνεται,
					//ενώ επιπλέον δεν υφίσταται ανάγκη περαιτέρω ελέγχου ομοιότητας της στήλης i με άλλες στήλες j.
                    if (is_identical_stack)
                    {
                        identical_stacks++;
                        break;
                    }
                }
            }

			//Μετά το πέρας του δεύτερου βρόχου, αν το πλήθος των όμοιων στηλών δεν έχει μεταβληθεί,
			//σημαίνει πως δεν υπάρχει στήλη πανομοιότυπη της i στην κατάσταση του προγόνου,
			//και συνεπώς οι καταστάσεις των κυρίως στηλών τους, και, κατ' επέκταση, των ταμπλό τους δεν είναι πανομοιότυπες,
			//ειδάλλως η current_identical_stacks λαμβάνει τη νέα τιμή της identical_stacks.
            if (current_identical_stacks==identical_stacks)
                return 0;
            else
                current_identical_stacks=identical_stacks;
        }
	}

	//Εάν βγει από το loop η εκτέλεση, σημαίνει πως υπάρχει 1-1 αντιστοιχία
	return 1;
}

// Η συνάρτηση υπολογίζει το βάρος του νέου κόμβου-παιδιού
// Inputs:
//		struct tree_node *child: Το νέο παιδί
//		int method: Ο αλγόριθμος που επιλέχθηκε για την αναζήτηση της λύσης.
// Output:
//		Κατ' αναφορά: Το παιδί, με αλλαγές στο βάρος του
void calc_child_node_value(struct tree_node *child, int method)
{
    if (method==BEST)
        child->f=heuristic(child->board_state);
    else if (method==A_STAR)
        child->f=child->g + heuristic(child->board_state);
    else
        child->f=0;
}

// Με δεδομένο ένα κόμβο-φύλλο που αποτελεί λύση, η συνάρτηση υπολογίζει τις κινήσεις των καρτών που οδηγούν στην κατάσταση-στόχο από τη ρίζα.
// Inputs:
//		struct tree_node *solution_node	: A leaf-node
// Output:
//		Καθολικές μεταβλητές: Η σειρά των κινήσεων των καρτών από τη ρίζα ως την κατάσταση-στόχο στην καθολική μεταβλητή solution
void extract_solution(struct tree_node *solution_node)
{
	int i;

	struct tree_node *temp_node=solution_node;
	solution_length = solution_node->g;

    solution = (solution_step_t*) malloc(solution_length*sizeof(solution_step_t));

	temp_node=solution_node;
	i=solution_length;
	while (temp_node->parent!=NULL)
	{
		i--;
		solution[i].move_type = temp_node->card_movement.move_type;
		solution[i].card_moved = temp_node->card_movement.card_moved;
		solution[i].card_below = temp_node->card_movement.card_below;
		temp_node=temp_node->parent;
	}
}

// This function writes the solution into a file
// Inputs:
//		char* filename	: The name of the file where the solution will be written.
// Outputs:
//		Nothing (apart from the new file)
void write_solution_to_file(char* filename)
{
    int i;
    FILE *fout;
    fout=fopen(filename,"w");
    if (fout==NULL)
    {
        printf("Cannot open output file to write solution.\n");
        printf("Now exiting...");
        return;
    }
    fprintf(fout,"%d\n",solution_length);
    for (i=0;i<solution_length;i++)
        switch(solution[i].move_type)
        {
            case STACK:
                fprintf(fout,"stack ");
                solution_step_text(i, fout, 1);
                break;

            case FREECELL:
                fprintf(fout,"freecell ");
                solution_step_text(i, fout, 0);
                break;

            case FOUNDATION:
                fprintf(fout,"source ");
                solution_step_text(i, fout, 0);
                break;

            case NEW_STACK:
                fprintf(fout,"new_stack ");
                solution_step_text(i, fout, 0);
                break;
        }
    fclose(fout);
}

void solution_step_text(int step, FILE *fout, int is_stack_to_stack_move)
{
	if (solution[step].card_moved->suit == HEARTS)
		fprintf(fout, "H");
	else if (solution[step].card_moved->suit == DIAMONDS)
		fprintf(fout, "D");
	else if (solution[step].card_moved->suit == SPADES)
		fprintf(fout, "S");
	else if (solution[step].card_moved->suit == CLUBS)
		fprintf(fout, "C");

	if (is_stack_to_stack_move) //Μόνο για την περίπτωση όπου η κίνηση είναι από stack σε stack και απαιτούνται 2 τιμές στη σειρά
	{
		fprintf(fout, "%d ", solution[step].card_moved->value);
		solution_step_text(step, fout, 0);
	}
	else if (solution[step].card_below!=NULL) //Μόνο για την περίπτωση όπου η κίνηση είναι από stack σε stack και απαιτούνται 2 τιμές στη σειρά
		fprintf(fout, "%d \n", solution[step].card_below->value);
    else
        fprintf(fout, "%d \n", solution[step].card_moved->value);

    return;
}


//NOT CHANGED

// Auxiliary function that displays a message in case of wrong input parameters.
void syntax_message()
{
	printf("puzzle <method> <input-file> <output-file>\n\n");
	printf("where: ");
	printf("<method> = breadth|depth|best|astar\n");
	printf("<input-file> is a file containing a freecell game\n");
	printf("<output-file> is the file where the solution will be written.\n");
}

// Reading run-time parameters.
int get_method(char* s)
{
	if (strcmp(s,"breadth")==0)
		return BREADTH;
	else if (strcmp(s,"depth")==0)
		return DEPTH;
	else if (strcmp(s,"best")==0)
		return BEST;
	else if (strcmp(s,"astar")==0)
		return A_STAR;
	else
		return -1;
}

// This function adds a pointer to a new leaf search-tree node at the front of the frontier.
// This function is called by the depth-first search algorithm.
// Inputs:
//		struct tree_node *node	: A (leaf) search-tree node.
// Output:
//		0 --> The new frontier node has been added successfully.
//		-1 --> Memory problem when inserting the new frontier node .
int add_frontier_front(struct tree_node *node)
{
	// Creating the new frontier node
	struct frontier_node *new_frontier_node=(struct frontier_node*)
                                malloc(sizeof(struct frontier_node));
	if (new_frontier_node==NULL)
		return -1;

	new_frontier_node->n = node;
	new_frontier_node->previous = NULL;
	new_frontier_node->next = frontier_head;

	if (frontier_head==NULL)
	{
		frontier_head=new_frontier_node;
		frontier_tail=new_frontier_node;
	}
	else
	{
		frontier_head->previous=new_frontier_node;
		frontier_head=new_frontier_node;
	}

#ifdef SHOW_COMMENTS
	printf("Added to the front...\n");
	//display_puzzle(node->p);
#endif
	return 0;
}

// This function adds a pointer to a new leaf search-tree node at the back of the frontier.
// This function is called by the breadth-first search algorithm.
// Inputs:
//		struct tree_node *node	: A (leaf) search-tree node.
// Output:
//		0 --> The new frontier node has been added successfully.
//		-1 --> Memory problem when inserting the new frontier node .
int add_frontier_back(struct tree_node *node)
{
	// Creating the new frontier node
	struct frontier_node *new_frontier_node=(struct frontier_node*) malloc(sizeof(struct frontier_node));
	if (new_frontier_node==NULL)
		return -1;

	new_frontier_node->n=node;
	new_frontier_node->next=NULL;
	new_frontier_node->previous=frontier_tail;

	if (frontier_tail==NULL)
	{
		frontier_head=new_frontier_node;
		frontier_tail=new_frontier_node;
	}
	else
	{
		frontier_tail->next=new_frontier_node;
		frontier_tail=new_frontier_node;
	}

#ifdef SHOW_COMMENTS
	printf("Added to the back...\n");
	//display_puzzle(node->p);
#endif

	return 0;
}

// This function adds a pointer to a new leaf search-tree node within the frontier.
// The frontier is always kept in increasing order wrt the f values of the corresponding
// search-tree nodes. The new frontier node is inserted in order.
// This function is called by the heuristic search algorithm.
// Inputs:
//		struct tree_node *node	: A (leaf) search-tree node.
// Output:
//		0 --> The new frontier node has been added successfully.
//		-1 --> Memory problem when inserting the new frontier node .
int add_frontier_in_order(struct tree_node *node)
{
	// Creating the new frontier node
	struct frontier_node *new_frontier_node=(struct frontier_node*)
                malloc(sizeof(struct frontier_node));
	if (new_frontier_node==NULL)
		return -1;

	new_frontier_node->n=node;
	new_frontier_node->previous=NULL;
	new_frontier_node->next=NULL;

	if (frontier_head==NULL)
	{
		frontier_head=new_frontier_node;
		frontier_tail=new_frontier_node;
	}
	else
	{
		struct frontier_node *pt;
		pt=frontier_head;

		// Search in the frontier for the first node that corresponds to either a larger f value
		// or to an equal f value but larger h value
		// Note that for the best first search algorithm, f and h values coincide.
		while (pt!=NULL && (pt->n->f<node->f || (pt->n->f==node->f && pt->n->h<node->h)))
			pt=pt->next;

		if (pt!=NULL)
		{
			// new_frontier_node is inserted before pt .
			if (pt->previous!=NULL)
			{
				pt->previous->next=new_frontier_node;
				new_frontier_node->next=pt;
				new_frontier_node->previous=pt->previous;
				pt->previous=new_frontier_node;
			}
			else
			{
				// In this case, new_frontier_node becomes the first node of the frontier.
				new_frontier_node->next=pt;
				pt->previous=new_frontier_node;
				frontier_head=new_frontier_node;
			}
		}
		else
		{
			// if pt==NULL, new_frontier_node is inserted at the back of the frontier
			frontier_tail->next=new_frontier_node;
			new_frontier_node->previous=frontier_tail;
			frontier_tail=new_frontier_node;
		}
	}

#ifdef SHOW_COMMENTS
	printf("Added in order (f=%d)...\n",node->f);
	//display_puzzle(node->p);
#endif

	return 0;
}

//Εναλλακτική ευρετική
/* int find_highest_value_card_opportunities(board_state_t board_state, int *empty_stacks)
{
    *empty_stacks=0;
    int empty_freecells=0;
    int score=0;
    //Το σκορ υπολογίζεται με τους εξής τρόπους, ανάλογα με τη θέση των Ρηγών και των αντίστοιχων Νταμών (αντίστοιχα Ν-1 και Ν-2, εφόσον Ν>1)
    //1) Αν ο Ρήγας (ή το αντίστοιχο Ν-1) είναι ήδη στο βάθος στήλης...
    //  1α)... και είναι το μοναδικό φύλλο: 3 πόντοι + 2 πόντοι ανά φύλλο Ν-2 αντίθετου χρώματος σε κορυφή στήλης / +3 σε freecell
    //  1β)... και ακολουθείται από τουλάχιστον μία "καλή" ακολουθία: 5 πόντοι +2 πόντοι για κάθε φύλλο

    //2) Αν ο Ρήγας (ή το αντίστοιχο Ν-1) είναι στην κορυφή στήλης μήκους>1 ή σε freecell:
    //  2α)...και υπάρχει κενή στήλη: 4 πόντοι


    for (int i=0;i<MAIN_STACKS_NUM;i++)
    {
        int stack_i_size = board_state.main_stack_sizes[i];
        if (stack_i_size==0)
            empty_stacks++;
    }

    //Βήμα 1ο: Εντοπισμός
    for (int i=0;i<MAIN_STACKS_NUM;i++)
    {
        int stack_i_size = board_state.main_stack_sizes[i];

        //
        if (stack_i_size>0)
        {
            card_t stack_i_top_card = board_state.main_stack_cards[i][stack_i_size-1];
            card_t stack_i_bottom_card=board_state.main_stack_cards[i][0];

            //Περίπτωση 1: Το φύλλο με τιμή Ν-1 είναι ήδη στο βάθος μιας στοίβας
            //1α
            if (stack_i_size==1 && stack_i_top_card.value==N-1)
                score+=(3+bonus_score_for_complement_cards(board_state, stack_i_top_card));
            else if (stack_i_size>1 && stack_i_bottom_card.value==N-1)
            {
                score+=5;
                int chain_broken=0;
                int next_index=1;
                int next_expected_card_value=stack_i_bottom_card.value-1;
                int next_expected_card_color=(stack_i_bottom_card.color+1)%2;
                while (next_expected_card_value>-1 && next_index<stack_i_size && !chain_broken)
                {
                    next_index++;
                    card_t next_card = board_state.main_stack_cards[i][next_index];
                    if (next_card.value==next_expected_card_value && next_card.color==next_expected_card_color)
                    {
                        score+=2*(N-next_card.value);
                        int next_expected_card_value=next_expected_card_value-1;
                        int next_expected_card_color=(next_expected_card_color+1)%2;
                    }
                    else
                        chain_broken=1;
                }
                if (!chain_broken)
                    score+=(stack_i_top_card.value * bonus_score_for_complement_cards(board_state, stack_i_top_card));
            }
            else if (stack_i_size>1 && stack_i_top_card.value==N-1 && (*empty_stacks)>0)
            {
                score+=4;
            }
        }
    }

    empty_freecells=board_state.freecell_size<FREECELLS_NUM;
    if (empty_freecells)
    {
        for (int i=0;i<FREECELLS_NUM;i++)
            if (board_state.freecell_cards[i].value==N-1 && empty_stacks>0)
                score+=3;
    }

    return score;
}

int bonus_score_for_complement_cards(board_state_t board_state, card_t curr_stack_top_card)
{
    int top_card_value = curr_stack_top_card.value;
    int bonus_score=0;
    if (top_card_value>0)
    {
        int expected_value=top_card_value-1;
        int expected_color=(curr_stack_top_card.color+1)%2;
        int cards_to_find=2;

        if (board_state.freecell_size>0)
        {
            for (int i=0;i<FREECELLS_NUM;i++)
                if (board_state.freecell_cards[i].value==expected_value && board_state.freecell_cards[i].color==expected_color)
                {
                    bonus_score+=3;
                    cards_to_find--;
                }
        }

        if (!cards_to_find) return bonus_score;

        for (int i=0;i<MAIN_STACKS_NUM;i++)
        {
            if (board_state.main_stack_sizes[i]>0)
            {
                int stack_i_top_card_index = board_state.main_stack_sizes[i]-1;
                card_t stack_i_top_card = board_state.main_stack_cards[i][stack_i_top_card_index];

                if (stack_i_top_card.value==expected_value && stack_i_top_card.color==expected_color)
                {
                    bonus_score+=2;
                    cards_to_find--;

                    if (!cards_to_find) return bonus_score;
                }
            }
        }
    }
    return bonus_score;
}
 */
 
//ΣΥΝΑΡΤΗΣΕΙΣ DEBUG
void display_board_state(struct tree_node *current_node)
{
    //Node h
    //printf("Node evaluation: %d\n", current_node->f);
    //Display freecells
    for (int i=0;i<FREECELLS_NUM;i++)
        display_card(current_node->board_state.freecell_cards[i]);

    printf(" || ");

    //Display foundations
    for (int i=0;i<FOUNDATIONS_NUM;i++)
    {
        int top_card_index;
        if (current_node->board_state.foundation_card_sizes[i]==0)
            top_card_index=0;
        else
            top_card_index=current_node->board_state.foundation_card_sizes[i]-1;

        display_card(current_node->board_state.foundation_cards[i][top_card_index]);
    }

    //Display main stack
    printf("\n");
    for (int i=0;i<MAIN_STACKS_NUM;i++)
    {
        int stack_size = current_node->board_state.main_stack_sizes[i];
        for (int j=0;j<stack_size;j++)
            display_card(current_node->board_state.main_stack_cards[i][j]);

        printf("\n");
    }

    printf("--------------------------------------------------------\n");
}

void display_card(card_t card)
{
    if (card.value==-1)
    {
        printf(" -- ");
        return;
    }

    if (card.suit == HEARTS)
		printf("H");
	else if (card.suit == DIAMONDS)
		printf("D");
	else if (card.suit == SPADES)
		printf("S");
	else if (card.suit == CLUBS)
		printf("C");

    printf("%d  ", card.value);
    return;
}
