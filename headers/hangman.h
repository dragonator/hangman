#ifndef __HANGMAN_H__
#define __HANGMAN_H__

// Type definitions
typedef enum {
  UNKNOWN_GUESS_RESULT,
  CORRECT_LETTER_GUESS,
  WRONG_LETTER_GUESS,
  LETTER_ALREADY_GUESSED,
  CORRECT_WORD_GUESS,
  WRONG_WORD_GUESS,
} guess_result_type;

typedef enum {
  GAME_INITIALIZED,
  GAME_NOT_INITIALIZED,
  GAME_TERMINATED = GAME_NOT_INITIALIZED,
  GAME_NOT_TERMINATED,
  GAME_OVER = GAME_NOT_TERMINATED
} game_state_type;

typedef enum {
  USER_WON,
  USER_LOST,
  UNKNOWN_GAME_RESULT
} game_result_type;

// Variables
char  wrong_guesses;
char *used_letters;
char *word;
char *word_in_progress;
game_state_type  game_state;
game_result_type game_result;

// Functions
void initialize_game(const char  guesses_max,
                     const char  word_len_min,
                     const char *dictionary_file,
                     const char *guessed_words_file);
void terminate_game();
guess_result_type guess_letter(const char letter);
guess_result_type guess_word(const char *word);

#endif
