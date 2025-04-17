#include <stdio.h>

// Define the bit-board
#define U64 unsigned long long

//set get pop square
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

#define empty_board "8/8/8/8/8/8/8/8 w - -"
#define position "r3kbnr/p4ppp/n3bq2/8/8/8/PPPPPPPP/RNBQKBNR w K-k- c5  0 1"
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"

// Define the board
enum {
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

char *square_to_coordinates[]={
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

enum {
  WHITE,
  BLACK,
};

enum{
  rook,
  bishop
};

enum{
  P,N,B,R,Q,K,p,n,b,r,q,k
};

enum{wk=1,wq=2,bk=4,bq=8};

char ascii_pirces[] = "PNBRQKpnbrqk";

// unicode pieces
char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

// char pieces
int char_pieces[] = {
    [ 'P' ] = P, [ 'N' ] = N, [ 'B' ] = B, [ 'R' ] = R, [ 'Q' ] = Q, [ 'K' ] = K,
    [ 'p' ] = p, [ 'n' ] = n, [ 'b' ] = b, [ 'r' ] = r, [ 'q' ] = q, [ 'k' ] = k
};

U64 bitboards[12];

U64 occupancy[3];

int side = WHITE;

int enpassant = no_sq;

int castle = 0;


// Print the board
void print_bitboard(U64 bitboard) {
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      if(!file) printf(" %d   ", 8 - rank);
      if (get_bit(bitboard, square)) {
        printf("1 ");
      } else {
        printf("0 ");
      }
    }
    printf("\n");
  }
  printf("\n     a b c d e f g h\n");
}

//not A file const
const U64 file_a = 0xfefefefefefefefe;

//not H file const
const U64 file_h = 0x7f7f7f7f7f7f7f7f;

//not AB file const
const U64 file_ab = 0xfcfcfcfcfcfcfcfc;

//not GH file const
const U64 file_gh = 0x3f3f3f3f3f3f3f3f;

//no edge const 
const U64 no_edge = 0x007e7e7e7e7e7e00;

// bishop relevant occupancy bit count for every square on board
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};

// rook relevant occupancy bit count for every square on board
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};

//attack tables 
//pawn
U64 pawn_attacks[2][64];

//knight
U64 knight_attacks[64];

//king
U64 king_attacks[64];

//pseudo random num gen
unsigned int get_random_number(void){
  static unsigned int seed = 1804289383;
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;

  return seed;
}

//pawn_attack_function
U64 mask_pawn_attack(int side, int square) {
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;
  set_bit(bitboard, square);
  if (side == WHITE) {
    attacks |= (bitboard >> 9) & file_h;
    attacks |= (bitboard >> 7) & file_a;
  } else {
    attacks |= (bitboard << 9) & file_a;
    attacks |= (bitboard << 7) & file_h;
  }
  return attacks;
}

//knight_attack_function
U64 mask_knight_attack(int square) {
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;
  set_bit(bitboard, square);
  attacks |= (bitboard << 17) & file_a;
  attacks |= (bitboard << 15) & file_h;
  attacks |= (bitboard << 10) & file_ab;
  attacks |= (bitboard << 6) & file_gh;
  attacks |= (bitboard >> 17) & file_h;
  attacks |= (bitboard >> 15) & file_a;
  attacks |= (bitboard >> 10) & file_gh;
  attacks |= (bitboard >> 6) & file_ab;
  return attacks;
}

//king_attack_function
U64 mask_king_attack(int square) {
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;
  set_bit(bitboard, square);
  attacks |= (bitboard << 1) & file_a;
  attacks |= (bitboard >> 1) & file_h;
  attacks |= bitboard << 8;
  attacks |= bitboard >> 8;
  attacks |= (bitboard << 9) & file_a;
  attacks |= (bitboard << 7) & file_h;
  attacks |= (bitboard >> 9) & file_h;
  attacks |= (bitboard >> 7) & file_a;
  return attacks;
}

// count bits within a bitboard )
static inline int count_bits(U64 bitboard)
{
    // bit counter
    int count = 0;
    
    // consecutively reset least significant 1st bit
    while (bitboard)
    {
        // increment count
        count++;
        
        // reset least significant 1st bit
        bitboard &= bitboard - 1;
    }
    
    // return bit count
    return count;
}

// print board
void print_board()
{
    // print offset
    printf("\n");

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop ober board files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // print ranks
            if (!file)
                printf("  %d ", 8 - rank);
            
            // define piece variable
            int piece = -1;
            
            // loop over all piece bitboards
            for (int bb_piece = P; bb_piece <= k; bb_piece++)
            {
                // if there is a piece on current square
                if (get_bit(bitboards[bb_piece], square))
                    // get piece code
                    piece = bb_piece;
            }
            
            // print different piece set depending on OS
            #ifdef WIN64
                printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
            #else
                printf(" %s", (piece == -1) ? "." : unicode_pieces[piece]);
            #endif
        }
        
        // print new line every rank
        printf("\n");
    }
    
    // print board files
    printf("\n     a b c d e f g h\n\n");
    
    // print side to move
    printf("     Side:     %s\n", !side ? "white" : "black");
    
    // print enpassant square
    printf("     Enpassant:   %s\n", (enpassant != no_sq) ? square_to_coordinates[enpassant] : "no");
    
    // print castling rights
    printf("     Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-',
                                           (castle & wq) ? 'Q' : '-',
                                           (castle & bk) ? 'k' : '-',
                                           (castle & bq) ? 'q' : '-');
}

void parse_fen(const char *fen)
{
    // reset bitboards
    for (int index = 0; index < 12; index++)
        bitboards[index] = 0ULL;
    
    // reset game state
    side = 0;
    enpassant = no_sq;
    castle = 0;
    
    // reset board pointer
    int square = 0;
    
    // parse FEN string
for (int square = 0; square < 64 && *fen && *fen != ' '; )
    {
        // match ascii pieces within FEN string
        if ((*fen >= 'b' && *fen <= 'r') || (*fen >= 'B' && *fen <= 'R'))
        {
            // init piece type
            int piece = char_pieces[*fen];
            
            // set piece on corresponding bitboard
            set_bit(bitboards[piece], square);
            
            // increment square and pointer to FEN string
            square++;
            fen++;
        }
        
        // match empty square numbers within FEN string
        else if (*fen >= '1' && *fen <= '8')
        {
            // init offset (convert char 0 to int 0)
            int offset = *fen - '0';
            
            // increment square and pointer to FEN string
            square += offset;
            fen++;
        }
        
        // match rank separator
        else if (*fen == '/')
        {
            // increment pointer to FEN string
            fen++;
        }
        else
        {
            fen++; // error
        }
    }
  *fen++;

  //parse side
  if(*fen == 'w') side = WHITE;
  else side = BLACK;
  fen+=2;

  //parse castle
  while(*fen != ' '){
    switch(*fen){
      case 'K': castle |= wk; break;
      case 'Q': castle |= wq; break;
      case 'k': castle |= bk; break;
      case 'q': castle |= bq; break;
    }
    fen++;
  }

  fen++;

  //parse en passant
  if(*fen != '-'){
    int file = fen[0] - 'a';
    int rank = 8 - (fen[1] - '0');
    enpassant = rank*8+file;
  }
  
  //set occupancy bitboards
  occupancy[WHITE] = bitboards[P] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | bitboards[K];
  occupancy[BLACK] = bitboards[p] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q] | bitboards[k];
  occupancy[2] = occupancy[WHITE] | occupancy[BLACK];
}

// get least significant 1st bit index
static inline int get_ls1b_index(U64 bitboard)
{
    // make sure bitboard is not 0
    if (bitboard)
    {
        // count trailing bits before LS1B
        return count_bits((bitboard & -bitboard) - 1);
    }
    
    //otherwise
    else
        // return illegal index
        return -1;
}

  //bishop attack function
U64 mask_bishop_attack(int square, U64 block){
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;
  U64 file_mask_left = 0ULL;
  U64 rank_mask_down = 0ULL;
  U64 file_mask_right = 0ULL;
  U64 rank_mask_up = 0ULL;
  int rank = square/8;
  int file = square%8;
  for(int r=0;r<8;r++){
    for(int f=0;f<8;f++){
      int s=r*8+f;
      if(f<file){
        set_bit(file_mask_left, s);
      }
      if(f>file){
        set_bit(file_mask_right, s);
      }
      if(r<rank){
        set_bit(rank_mask_down, s);
      }
      if(r>rank){
        set_bit(rank_mask_up, s);
      }
    }
  }
  //print_bitboard(file_mask_left);
  //print_bitboard(file_mask_right);
  //print_bitboard(rank_mask_down);
  //print_bitboard(rank_mask_up);
  set_bit(bitboard, square);
  for(int i=1;i<8;i++){
    attacks |= (bitboard << (i*9)) & ~file_mask_left ;
    if(get_bit(block, square+i*9)){
      break;
    }
  }
  for(int i=1;i<8;i++){
    attacks |= (bitboard << (i*7)) & ~file_mask_right ;
    if(get_bit(block, square+i*7)){
      break;
    }
  }for(int i=1;i<8;i++){
    attacks |= (bitboard >> (i*9)) & ~file_mask_right ;
    if(get_bit(block, square-i*9)){
      break;
    }
  }for(int i=1;i<8;i++){
    attacks |= (bitboard >> (i*7)) & ~file_mask_left ;
    if(get_bit(block, square-i*7)){
      break;
    }
  }
  return attacks&no_edge;
}

//rook attack function
U64 mask_rook_attack(int square, U64 block){
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;
  U64 file_mask_left = 0ULL;
  U64 rank_mask_down = 0ULL;
  U64 file_mask_right = 0ULL;
  U64 rank_mask_up = 0ULL;
  int rank = square/8;
  int file = square%8;
  for(int r=0;r<8;r++){
    for(int f=0;f<8;f++){
      int s=r*8+f;
      if(f<file){
        set_bit(file_mask_left, s);
      }
      if(f>file){
        set_bit(file_mask_right, s);
      }
      if(r<rank){
        set_bit(rank_mask_down, s);
      }
      if(r>rank){
        set_bit(rank_mask_up, s);
      }
    }
  }
  //print_bitboard(file_mask_left);
  //print_bitboard(file_mask_right);
  //print_bitboard(rank_mask_down);
  //print_bitboard(rank_mask_up);
  set_bit(bitboard, square);
  for(int i=1;i<8;i++){
    attacks |= (bitboard << (i*8)) ;
    if(get_bit(block, square+i*8)){
      break;
    }
  }
  for(int i=1;i<8;i++){
    attacks |= (bitboard >> (i*8)) ;
    if(get_bit(block, square-i*8)){
      break;
    }
  }for(int i=1;i<8;i++){
    attacks |= (bitboard << i) & ~file_mask_left ;
    if(get_bit(block, square+i)){
      break;
    }
  }for(int i=1;i<8;i++){
    attacks |= (bitboard >> i) & ~file_mask_right ;
    if(get_bit(block, square-i)){
      break;
    }
  }
  return attacks;
}

// set occupancies
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask)
{
    // occupancy map
    U64 occupancy = 0ULL;

    // loop over the range of bits within attack mask
    for (int count = 0; count < bits_in_mask; count++)
    {
        // get LS1B index of attacks mask
        int square = get_ls1b_index(attack_mask);

        // pop LS1B in attack map
        pop_bit(attack_mask, square);

        // make sure occupancy is on board
        if (index & (1 << count))
            // populate occupancy map
            occupancy |= (1ULL << square);
    }

    // return occupancy map
    return occupancy;
}

//init leaper attacks 
void init_leaper_attacks(){
for(int square=0;square<64;square++){
    pawn_attacks[WHITE][square] = mask_pawn_attack(WHITE, square);
    pawn_attacks[BLACK][square] = mask_pawn_attack(BLACK, square);
    knight_attacks[square] = mask_knight_attack(square);
    king_attacks[square] = mask_king_attack(square);
 }
}



int main() {
  parse_fen(position);
  //parse_fen(empty_board);
  //parse_fen(start_position);
  print_board();
  init_leaper_attacks();
 //print_bitboard(mask_pawn_attack(WHITE, a2));
 //printf(" WHITE a2\n");
 //print_bitboard(mask_pawn_attack(WHITE, h4));
 //printf(" WHITE h4\n");
 //print_bitboard(mask_pawn_attack(BLACK, c7));
 //printf(" BLACK c7\n");
 //print_bitboard(mask_pawn_attack(BLACK, a2));
 //printf(" BLACK a2\n");
 U64 bitboard = 0ULL;
 //set_bit(bitboard, a1);
 //set_bit(bitboard, b2);
 //set_bit(bitboard, e3);
 //set_bit(bitboard, c5);
 //printf("Initial Board\n");
 //printf("%ud\n",get_random_number());
 //printf("%ud\n",get_random_number());
 //printf("%ud\n",get_random_number());
 //printf("%ud\n",get_random_number());
 //print_bitboard(bitboard);
 //for(int rank=0;rank<8;rank++){
   //for(int file=0;file<8;file++){
     //int square = rank*8+file;
     //if(!((rank==0)||(rank==7))){
       //set_bit(bitboard, square);
     //}
   //}
 //}

  //print_bitboard(mask_bishop_attack(h6));
  //print_bitboard(~file_ab);
  //printf("%llx\n",~file_ab);
  //print_bitboard(bitboard&file_a&file_h);
  //printf("%llx\n",bitboard&file_a&file_h);
  //printf("%llx\n",~file_gh);
  //print_bitboard(mask_bishop_attack(b1));
  //print_bitboard(mask_rook_attack(c3,bitboard));
  //print_bitboard(set_occupancy(95,count_bits(mask_bishop_attack(d4,bitboard)),mask_bishop_attack(d4,bitboard)));
  //print_bitboard((U64)1<<63);
  //printf("%llx\n",bitboard);
  //printf("%d\n",count_bits(524288));
  //printf("%d\n",count_bits(8));
  //printf("%s\n",board[d4]);
  return 0;
}

