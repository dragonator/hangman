#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include "../headers/hangman.h"
#include <signal.h>

#include <signal.h>

// Macro definitions
#define abort_game() terminate_game()
#define report_errno() \
      char command[PATH_MAX+22]; \
      sprintf(command, "head -%d %s | tail -1", __LINE__-1, __FILE__); \
      fprintf(stderr, "An error occured. (Errno: %d)\n", errno);     \
      fprintf(stderr, "File: %s\nFunc: %s\nLine: %d\n",              \
                     __FILE__, __func__, __LINE__-1);                \
      system(command);

// Static variables
static char  _guesses_max        = 0;
static char *_dictionary_file    = NULL;
static char *_guessed_words_file = NULL;

// Static function declarations
static char* _get_random_word(const char  word_len_min,
                              const char *dictionary_file,
                              const char *guessed_words_file);
static void  _add_to_guessed_words(const char *word,
                                   const char *guessed_words_file);

// Static wrapper functions around builtin ones
static void* _my_malloc(size_t size);
static void* _my_fopen_r(const char *filename);

// Public function definitions
void initialize_game(const char  guesses_max,
                     const char  word_len_min,
                     const char *dictionary_file,
                     const char *guessed_words_file)
{
  size_t word_len = 0;

  if(GAME_NOT_TERMINATED == game_state)
    terminate_game();

  // Letters in alphabet + '\0' = 27
  if(!(used_letters = _my_malloc(guesses_max + 27)))
    exit(EXIT_FAILURE);

  used_letters[0] = '\0';
  word = _get_random_word(word_len_min,
                          dictionary_file,
                          guessed_words_file);

  word_len = strlen(word);
  if(!(word_in_progress = _my_malloc(word_len+1)))
    exit(EXIT_FAILURE);

  memset(word_in_progress, '_', word_len);
  word_in_progress[word_len] = '\0';

  wrong_guesses = 0;
  game_state = GAME_INITIALIZED;
  _guesses_max = guesses_max;
  _dictionary_file    = (char*)dictionary_file;
  _guessed_words_file = (char*)guessed_words_file;
}

void terminate_game()
{
  free(used_letters);
  used_letters = NULL;

  free(word_in_progress);
  word_in_progress = NULL;

  free(word);
  word = NULL;

  game_state = GAME_TERMINATED;
  game_result = UNKNOWN_GAME_RESULT;
}

guess_result_type guess_letter(const char letter)
{
  // Make sure that letter is lowercase
  int used_letters_count         = 0;
  const char lower_letter        = tolower(letter);
  guess_result_type guess_result = UNKNOWN_GUESS_RESULT;

  // If letter in used_letters
  if(NULL != strchr(used_letters, letter))
    return LETTER_ALREADY_GUESSED;

  // Add to guessed letters
  used_letters_count = strlen(used_letters);
  used_letters[used_letters_count] = letter;
  used_letters[used_letters_count + 1] = '\0';

  //TEST: Transform word_in_progress to show guessed letters
  for(char *cur_letter = word ; *cur_letter ; cur_letter++){
    // If letter match
    if((lower_letter == *cur_letter) || (lower_letter == tolower(*cur_letter))){
      word_in_progress[cur_letter - word] = *cur_letter;
      guess_result = CORRECT_LETTER_GUESS;
    }
  }

  // If letter in word
  if(guess_result == CORRECT_LETTER_GUESS){
    //TEST: Check if word is completly guessed
    if(0 == strcmp(word, word_in_progress)){
      game_result  = USER_WON;
      game_state   = GAME_OVER;
      guess_result = CORRECT_WORD_GUESS;
    }

  }
  else { // If letter not in word
    wrong_guesses++;
    guess_result = WRONG_LETTER_GUESS;

    //TEST: Check if game is lost
    if(_guesses_max == wrong_guesses){
      game_result = USER_LOST;
      game_state  = GAME_OVER;
    }
  }

  return guess_result;
}

guess_result_type guess_word(const char* input_word)
{
  guess_result_type guess_result = UNKNOWN_GUESS_RESULT;

  if(0 == strcmp(word, input_word)){
    game_result  = USER_WON;
    game_state   = GAME_OVER;
    _add_to_guessed_words(word, _guessed_words_file);
    return CORRECT_WORD_GUESS;
  }

  wrong_guesses++;
  guess_result = WRONG_WORD_GUESS;

  //TEST: Check if game is lost
  if(_guesses_max == wrong_guesses){
    game_result = USER_LOST;
    game_state  = GAME_OVER;
  }

  return guess_result;
}

// Static function definition

// Returns randomly picked word from the dictionary file.
// Dubplicated lines from dictionary_file and guessed_words_file
// are avoided, if guessed_words_file exists.
static char* _get_random_word(const char word_len_min,
                             const char *dictionary_file,
                             const char *guessed_words_file)
{
  char   reselect_word     = 0;
  char  *guessed_word      = NULL;
  char  *random_word       = NULL;
  size_t random_word_len   = 0;
  size_t guessed_word_len  = 0;
  long   dict_file_size    = 0;
  int    random_line_pos   = 0;
  FILE  *gw_fp             = NULL;
  FILE  *dict_fp           = NULL;

  // Initialize random number generator
  srand(time(NULL));

  // Open file for reading
  if(access(guessed_words_file, F_OK) != -1) {
    gw_fp = _my_fopen_r(guessed_words_file);
  }

  dict_fp = _my_fopen_r(dictionary_file);
  fseek(dict_fp, 0L, SEEK_END);
  dict_file_size = ftell(dict_fp);
  if(dict_file_size == 0){
    fprintf(stderr, "Dictionary file is empty. Aborting.");
    abort_game();
    exit(EXIT_FAILURE);
  }

  do {
    // Restore initial value
    reselect_word = 0;

    // Pick a random position in file
    random_line_pos = rand() % dict_file_size;
    fseek(dict_fp, random_line_pos, SEEK_SET);

    // Truncate content until next newline char
    if(getline(&random_word, &random_word_len, dict_fp) == (ssize_t)-1){
      report_errno();
      free(random_word);
      abort_game();
      exit(EXIT_FAILURE);
    }

    // Go to beginning of the file if eof
    if(feof(dict_fp))
      rewind(dict_fp);

    // Save random word in random_word
    if(getline(&random_word, &random_word_len, dict_fp) == (ssize_t)-1){
      report_errno();
      free(random_word);
      abort_game();
      exit(EXIT_FAILURE);
    }

    // Check if min_size condition is met
    if(word_len_min > strlen(random_word)){
        reselect_word = 1;
        continue;
    }

    // Verify that the word wasn't alredy guessed
    while(gw_fp && getline(&guessed_word, &guessed_word_len , gw_fp)){
      if(0 == strcmp(guessed_word, random_word)){
        reselect_word = 1;
        rewind(gw_fp);
        break;
      }
    }
  } while(reselect_word);

  // Post actions
  free(guessed_word);
  fclose(dict_fp);
  if(gw_fp)
    fclose(gw_fp);

  // Remove newline from end of word (line)
  random_word[strlen(random_word)-2] = '\0';

  return random_word;
}

static void _add_to_guessed_words(const char *word,
                                  const char *guessed_words_file)
{
  //TODO: Implement add_to_guessed_words
  FILE *gw_fp = fopen(_guessed_words_file, "a");
  if(!gw_fp){
    fprintf(stderr, "Can't open file for writing: %s\n", guessed_words_file);
    return;
  }

  fprintf(gw_fp, "%s\n", word);
  fclose(gw_fp);
}

// Wrapper function definitions
static void* _my_malloc(size_t size)
{
  void *ptr = malloc(size);
  if(!ptr){
    fprintf(stderr, "Allocation of memory failed.");
    abort_game();
  }

  return ptr;
}

static void* _my_fopen_r(const char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Can't open file for reading: %s\n", filename);
    abort_game();
  }

  return fp;
}
