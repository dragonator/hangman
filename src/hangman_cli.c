#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include "../headers/hangman.h"
#include "../headers/gallows.h"
#include <signal.h>

int main(int argc, char *argv[])
{
  // Local variables
  char    restart           = 0;
  char    datadir[PATH_MAX] = {0};
  char   *user_input        = NULL;
  size_t  user_input_size   = 0; 
  ssize_t characters        = 0;
  guess_result_type guess   = UNKNOWN_GUESS_RESULT;

  // Game configuration
  char word_len_min = 4;
  char guesses_max  = 7;
  char gallows_file[PATH_MAX];
  char dictionary_file[PATH_MAX];
  char guessed_words_file[PATH_MAX];

  // Get relative path to dat directory
  strncpy(datadir, argv[0], sizeof(datadir));
  dirname(datadir);
  strcat(datadir, "/../dat/");

  // Construct path to gallows file
  strcpy(gallows_file, datadir);
  strcat(gallows_file, "/../dat/gallows.dat");

  // Construct path to dictionary file
  strncpy(dictionary_file, datadir, sizeof(datadir));
  strcat(dictionary_file, "dictionary.dat");

  // Construct path to guessedwords file
  strncpy(guessed_words_file, datadir, sizeof(datadir));
  strcat(guessed_words_file, "guessedwords.dat");

  do {
    // Start new game
    initialize_game(guesses_max,
                    word_len_min,
                    dictionary_file,
                    guessed_words_file);

    // Play game
    do {

      // User output
      printf("===========================\n");
      print_gallows(wrong_guesses, 7, 7, gallows_file);
      printf("Used letters   : %s\n", used_letters);
      printf("Wrong guesses  : %d\n", wrong_guesses);
      printf("Guess the word : %s\n", word_in_progress);
      printf("\n");
      printf("Guess letter   : ");
      
      // User input
      switch(characters = getline(&user_input, &user_input_size, stdin)){

        case -1:
          printf("There was an error reading the input. (Errno: %d)\n", errno);
          printf("Could you please try again.");
          free(user_input);
          user_input = NULL;

        case  1:
          guess = UNKNOWN_GUESS_RESULT;
          break;
          
        case  2:
          guess = guess_letter(user_input[0]);
          break;

        default:
          // Remove newline
          user_input[characters-1] = '\0';
          guess = guess_word(user_input);
          break;
      }

      switch(guess){

        case UNKNOWN_GUESS_RESULT:
          //continue
          break;

        case CORRECT_LETTER_GUESS:
          printf("Correct letter guess: '%c'\n", user_input[0]);
          break;

        case WRONG_LETTER_GUESS:
          printf("Wrong letter guess: '%c'\n", user_input[0]);
          break;

        case LETTER_ALREADY_GUESSED:
          printf("Letter already used: '%c'\n", user_input[0]);
          break;

        case CORRECT_WORD_GUESS:
          printf("Correct word guess: '%s'\n", user_input);
          break;

        case WRONG_WORD_GUESS:
          printf("Wrong word guess: '%s'\n", user_input);
          break;
      }

      printf("===========================\n");

    } while(game_state != GAME_OVER);

    if(user_input != NULL)
      free(user_input);

    if(USER_WON == game_result)
      printf("Congratulations! You won!\n");
    else if(USER_LOST == game_result)
      printf("Game over! You lost!\n");
    else // game_result == UNKNOWN_GAME_RESULT
      printf("There was a problem during execution. Result unknown!\n");
    
    printf("The word was '%s'\n", word);

    terminate_game();

    printf("Restart? (y/n) : ");
    restart = getchar();
    while('y' != restart && 'n' != restart) {
      printf("Invalid input! Try again.\n");
      printf("Restart? (y/n) : ");
      restart = getchar();
    } 
  } while('y' == restart);

  return 0;
}
