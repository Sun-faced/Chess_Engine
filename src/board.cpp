#include <sstream>
#include "board.hpp"

static constexpr int BOARD_SIZE = 8;
static constexpr int PIECE_SECTION_IDX = 0;
static constexpr int SIDE_SECTION_IDX = 1;
static constexpr int CASTLING_SECTION_IDX = 2;
static constexpr int EN_PASSANT_SECTION_IDX = 3;
static constexpr int FIFTY_RULE_SECTION_IDX = 4;
static constexpr char PIECE_SYMBOLS[12] = {
  'P','N','B','R','Q','K',
  'p','n','b','r','q','k'
};

static int get_square(int rank, int file) {
  return rank * BOARD_SIZE + file;
}

static int char_to_piece_idx(char c) {
  int idx = 0;
  
  if (c >= 'a' && c <= 'z') {
    idx += NUMBER_OF_UNIQUE_PIECES;
    c = c - 'a' + 'A';
  }

  switch(c) {
    case 'P': return idx + 0;
    case 'N': return idx + 1;
    case 'B': return idx + 2;
    case 'R': return idx + 3;
    case 'Q': return idx + 4;
    case 'K': return idx + 5;
    default: return -1;
  }
}

static int get_piece_index(PieceType piece, Side side) {
  return static_cast<int>(piece) + NUMBER_OF_UNIQUE_PIECES * static_cast<int>(side);
}

static std::vector<std::string> split_string(const std::string& str, char delimiter) {
  std::vector<std::string> result;
  std::stringstream ss(str);
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    result.push_back(item);
  }
  return result;
}

std::string sq_to_string(int square) {
  int file = square % BOARD_SIZE;
  int rank = square / BOARD_SIZE;

  std::string result;
  result += ('a' + file);
  result += ('1' + rank);
  return result;
}

void Board::reset() {
  piece_location.fill(bb{0});
  occupancies.fill(bb{0});
  side = Side::white;
  enpassant = {false, -1};
  castling = 0;
  hash_key = bb{0};
  rep_idx = 0;
  fifty = 0;
}

uint64_t Board::generate_hash_key() {
  uint64_t result = 0ull;

  for (int piece = 0; piece < NUMBER_OF_PIECES; piece++) {
    bb temp = piece_location[piece];
    while (temp.get_board() != 0ull) {
      int square = temp.get_lsb_index();
      result ^= zobrist_table.piece_keys[piece][square];
      temp.pop_bit(square);
    }
  }

  if (enpassant.first) {
    result ^= zobrist_table.enp_keys[enpassant.second];
  }

  result ^= zobrist_table.castle_keys[castling];

  if (side == Side::black) {
    result ^= zobrist_table.side_key;
  }

  return result;
}

void Board::load_fen(const std::string& fen) { 
  reset();

  const std::vector<std::string> fen_sections = split_string(fen, ' ');
  const std::string& piece_section = fen_sections[PIECE_SECTION_IDX];
  const std::string& side_section = fen_sections[SIDE_SECTION_IDX];
  const std::string& castling_section = fen_sections[CASTLING_SECTION_IDX];
  const std::string& en_passant_section = fen_sections[EN_PASSANT_SECTION_IDX];
  const std::string& fifty_section = fen_sections[FIFTY_RULE_SECTION_IDX];

  int rank = 0;
  int file = 0;

  for (char c : piece_section) {
    if (c == '/') {
      rank++;
      file = 0;
      continue;
    }

    if (isdigit(c)) {
      file += c - '0';
      continue;
    }

    const int square = get_square(rank, file);
    const int piece_idx = char_to_piece_idx(c);

    piece_location[piece_idx].set_bit(square);
    occupancies[piece_idx / NUMBER_OF_UNIQUE_PIECES].set_bit(square);
    occupancies[static_cast<int>(Side::any)].set_bit(square);

    file++;
  }

  side = (side_section == "w") ? Side::white : Side::black;

  if (castling_section.find('K') != std::string::npos) {
    castling |= CastlingRights::white_king;
  }
  if (castling_section.find('Q') != std::string::npos) {
    castling |= CastlingRights::white_queen;
  }
  if (castling_section.find('k') != std::string::npos) {
    castling |= CastlingRights::black_king;
  }
  if (castling_section.find('q') != std::string::npos) {
    castling |= CastlingRights::black_queen;
  }

  if (en_passant_section == "-") {
    enpassant = {false, -1};
  } else {
    const uint32_t rank = en_passant_section[1] - '1';
    const uint32_t file = en_passant_section[0] - 'a';
    enpassant = {true, TOTAL_SQUARES - get_square(rank, file)};
  }

  hash_key = bb(generate_hash_key());

  fifty = std::stoi(fifty_section);
}

bool Board::is_sq_attacked(int square, Side attacking_side) const {
  const int defending_side_int = static_cast<int>((attacking_side == Side::white) ? Side::black : Side::white);
  const bb& all_pieces = occupancies[static_cast<int>(Side::any)];

  const bb attacking_pawns = piece_location[get_piece_index(PieceType::pawn, attacking_side)];
  if (attacking_pawns.get_board() & pawn_masks[defending_side_int][square].get_board()) {
    return true;
  }

  const bb attacking_knights = piece_location[get_piece_index(PieceType::knight, attacking_side)];
  if (attacking_knights.get_board() & knight_masks[square].get_board()) {
    return true;
  }

  const bb attacking_bishops = piece_location[get_piece_index(PieceType::bishop, attacking_side)];
  if (attacking_bishops.get_board() & get_bishop_attacks(square, all_pieces).get_board()) {
    return true;
  }

  const bb attacking_rooks = piece_location[get_piece_index(PieceType::rook, attacking_side)];
  if (attacking_rooks.get_board() & get_rook_attacks(square, all_pieces).get_board()) {
    return true;
  }

  const bb attacking_queens = piece_location[get_piece_index(PieceType::queen, attacking_side)];
  if (attacking_queens.get_board() & get_queen_attacks(square, all_pieces).get_board()) {
    return true;
  }

  const bb attacking_king = piece_location[get_piece_index(PieceType::king, attacking_side)];
  if (attacking_king.get_board() & king_masks[square].get_board()) {
    return true;
  }

  return false;
}

void Board::print() const {
  std::cout << "\n";

  for (int rank = 0; rank < BOARD_SIZE; rank++) {
    for (int file = 0; file < BOARD_SIZE; file++) {
      int square = get_square(rank, file);

      if (file == 0) {
        std::cout << "  " << BOARD_SIZE - rank << " ";
      }

      int piece = -1;
      for (int piece_type = 0; piece_type < NUMBER_OF_PIECES; piece_type++) {
        if (piece_location[piece_type].get_bit(square)) {
          piece = piece_type;
          break;
        }
      }

      std::cout << " " << (piece == -1 ? '.' : PIECE_SYMBOLS[piece]);
    }
    std::cout << "\n";
  }

  std::cout << "\n     a b c d e f g h\n\n";
  std::cout << "     Side:      " << (side == Side::white ? "white" : "black") << "\n";

  std::cout << "     Enpassant: ";
  if (enpassant.first) {
    int ep_square = enpassant.second;
    int ep_rank = ep_square / BOARD_SIZE;
    int ep_file = ep_square % BOARD_SIZE;
    char file_char = 'a' + ep_file;
    char rank_char = '1' + ep_rank;
    std::cout << file_char << rank_char << "\n";
  } else {
    std::cout << "no\n";
  }

  std::cout << "     Castling:  "
            << ((castling & CastlingRights::white_king)  ? 'K' : '-')
            << ((castling & CastlingRights::white_queen) ? 'Q' : '-')
            << ((castling & CastlingRights::black_king)  ? 'k' : '-')
            << ((castling & CastlingRights::black_queen) ? 'q' : '-')
            << "\n";
}

void Board::print_insides() {
  for (int i = 0; i < NUMBER_OF_PIECES; i++) {
    std::cout << piece_location[i].get_board() << std::endl;
  }

  for (int i = 0; i < NUMBER_OF_SIDES; i++) {
    std::cout << occupancies[i].get_board() << std::endl;
  }
  
  std::cout << static_cast<int>(side) << std::endl;

  if (enpassant.first) {
    std::cout << enpassant.second << std::endl;
  } else {
    std::cout << 64 << std::endl;
  }
}

void Board::move_piece(int piece_idx, int from_sq, int to_sq) {
  piece_location[piece_idx].pop_bit(from_sq);
  piece_location[piece_idx].set_bit(to_sq);

  hash_key ^= zobrist_table.piece_keys[piece_idx][from_sq];
  hash_key ^= zobrist_table.piece_keys[piece_idx][to_sq];
  
  // Update occupancies - determine which side this piece belongs to
  Side piece_side = (piece_idx < NUMBER_OF_UNIQUE_PIECES) ? Side::white : Side::black;
  
  occupancies[static_cast<int>(piece_side)].pop_bit(from_sq);
  occupancies[static_cast<int>(piece_side)].set_bit(to_sq);
  occupancies[static_cast<int>(Side::any)].pop_bit(from_sq);
  occupancies[static_cast<int>(Side::any)].set_bit(to_sq);
}

void Board::pop_piece(int sq, Side side) {
  const int start_idx = static_cast<int>(side) * NUMBER_OF_UNIQUE_PIECES;
  const int end_idx = start_idx + NUMBER_OF_UNIQUE_PIECES;

  for (int idx = start_idx; idx < end_idx; ++idx) {
    bb& bitboard = piece_location[idx];
    if (bitboard.get_bit(sq)) {
      bitboard.pop_bit(sq);
      hash_key ^= zobrist_table.piece_keys[idx][sq];
      
      occupancies[static_cast<int>(side)].pop_bit(sq);
      occupancies[static_cast<int>(Side::any)].pop_bit(sq);
      return;
    }
  }
}

const int castling_rights[64] = {
  7, 15, 15, 15,  3, 15, 15, 11,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  13, 15, 15, 15, 12, 15, 15, 14
};

bool Board::make_move(Move move) {
  Board board_cpy(*this);

  const Side moving_side = move.get_side_of_piece();
  const Side opposite_side_val = opposite_side(moving_side);
  const uint32_t from_sq = move.get_from_sq();
  const uint32_t to_sq = move.get_to_sq();
  const int piece_idx = static_cast<int>(move.get_piece()) + NUMBER_OF_UNIQUE_PIECES * static_cast<int>(moving_side);

  bool is_pawn_move = move.get_piece() == PieceType::pawn;
  bool resets_fifty_counter = is_pawn_move || move.is_capture();

  if (move.is_capture() && move.is_enpassant()) {
    const int captured_pawn_sq = to_sq + 8 - 16 * static_cast<int>(moving_side);

    pop_piece(captured_pawn_sq, opposite_side_val);
    occupancies[static_cast<int>(opposite_side_val)].pop_bit(captured_pawn_sq);
    occupancies[static_cast<int>(Side::any)].pop_bit(captured_pawn_sq);
  }

  if (move.is_capture() && !move.is_enpassant()) {

    occupancies[static_cast<int>(opposite_side_val)].pop_bit(to_sq);
    occupancies[static_cast<int>(Side::any)].pop_bit(to_sq);
    
    pop_piece(to_sq, opposite_side_val);
  }

  piece_location[piece_idx].pop_bit(from_sq);
  piece_location[piece_idx].set_bit(to_sq);

  occupancies[static_cast<int>(moving_side)].pop_bit(from_sq);
  occupancies[static_cast<int>(moving_side)].set_bit(to_sq);
  occupancies[static_cast<int>(Side::any)].pop_bit(from_sq);
  occupancies[static_cast<int>(Side::any)].set_bit(to_sq);

  hash_key ^= zobrist_table.piece_keys[piece_idx][from_sq];
  hash_key ^= zobrist_table.piece_keys[piece_idx][to_sq];

  if (move.is_promo()) {

    piece_location[piece_idx].pop_bit(to_sq);
    hash_key ^= zobrist_table.piece_keys[piece_idx][to_sq];

    const int prom_idx = static_cast<int>(move.get_prom_piece()) + NUMBER_OF_UNIQUE_PIECES * static_cast<int>(move.get_prom_side());
    piece_location[prom_idx].set_bit(to_sq);
    hash_key ^= zobrist_table.piece_keys[prom_idx][to_sq];

  }

  if (move.is_castle()) {
    const int rook_idx = static_cast<int>(PieceType::rook) + NUMBER_OF_UNIQUE_PIECES * static_cast<int>(moving_side);
    int rook_to_sq, rook_from_sq;
    
    if (to_sq % 8 == 6) {  // King-side castle
      rook_from_sq = to_sq + 1;
      rook_to_sq = to_sq - 1;
    } else {  // Queen-side castle
      rook_from_sq = to_sq - 2;
      rook_to_sq = to_sq + 1;
    }
    
    piece_location[rook_idx].pop_bit(rook_from_sq);
    piece_location[rook_idx].set_bit(rook_to_sq);
    
    // Update occupancies for rook movement
    occupancies[static_cast<int>(moving_side)].pop_bit(rook_from_sq);
    occupancies[static_cast<int>(moving_side)].set_bit(rook_to_sq);
    occupancies[static_cast<int>(Side::any)].pop_bit(rook_from_sq);
    occupancies[static_cast<int>(Side::any)].set_bit(rook_to_sq);
    
    // Update hash for rook movement
    hash_key ^= zobrist_table.piece_keys[rook_idx][rook_from_sq];
    hash_key ^= zobrist_table.piece_keys[rook_idx][rook_to_sq];
  }

  if (enpassant.first) {
    hash_key ^= zobrist_table.enp_keys[enpassant.second];
  }
  enpassant.first = false;

  if (move.is_double_pawn()) {
    const int enpassant_sq = to_sq + 8 - 16 * static_cast<int>(moving_side);
    enpassant.first = true;
    enpassant.second = enpassant_sq;
    hash_key ^= zobrist_table.enp_keys[enpassant_sq];
  }

  hash_key ^= zobrist_table.castle_keys[castling];
  castling &= castling_rights[from_sq];
  castling &= castling_rights[to_sq];
  hash_key ^= zobrist_table.castle_keys[castling];

  hash_key ^= zobrist_table.side_key;
  side = opposite_side(side);
  hash_key ^= zobrist_table.side_key;

  if (resets_fifty_counter) {
    fifty = 0;
  } else {
    fifty++;
  }

  // Check if the move leaves our king in check (illegal move)
  const Side our_side = opposite_side(side);
  const int king_idx = get_piece_index(PieceType::king, our_side);
  
  const bb king_bb = piece_location[king_idx];
  const int king_square = king_bb.get_lsb_index();
  if (is_sq_attacked(king_square, side)) {
    *this = board_cpy;
    return false;
  }

  return true;
}
