
```FileParser```:
Responsible for reading in a file, checking for errors, and parse out movies.


```Movie```:
A struct holding all the Movie info (id, title, type, genre, etc.)


```MovieIndex```:
A hashtable that indexes movies according to specified field. Basically wraps a hashtable with functionality specific to adding/removing/destroying with MovieSets. The key is the description for the document set, and the value is the document set.


```MovieReport```:
Responsible for printing out a report given a MovieIndex, to the command line or file.


```MovieSet```:
A set of Movies that are connected somehow: they have the same value for a given field. If the field is Year, all the movies in the set are made in the same year. It consists of a description and a LinkedList of Movies.

The basic tasks you need to complete:

* Modify main to accept flags on input and do the right thing.
* Modify Movie struct to hold an array of genres
* Modify MovieSet to ensure no duplicates of movies
* Bulk up tests

To do this, go through each file, and identify the "TODO" tags (listed below; generated with ```grep -n TODO *.c```). 

## The TODOs and some more details

To make sure you've implemented everything, you can run the following line of code: 

```
grep -nr TODO *
```

Note: some will require you to implement some code, while others will require you to answer a question in your writeup (README.md). 

## To run a single Google test

Instead of running all the tests at once, you can run just a single test something like this: 

```
./test_suite --gtest_filter=Hashtable.AddOneRemoveOne
```

Here, only the ```Hashtable.AddOneRemoveOne``` test will run. 

### Answers for TODOs questions

By Suicheng Su
CS5007 Summer 2019 A8

* FileParser.c.35: // TODO: How big to make this hashtable? How to decide? What to think about?
// Answer:	The appropriate size should be the size that allows the hashtable maintain O(1) look up 75% of the time. Therefore, size of 130 is about appropriate here.

* FileParser.c.57: // TODO: What to do if there is a collision? How might we change the system to help us deal with collision?
// Answer:	A collision is not expected to happen here. And ignoring the fact that AddMovieToIndex will not return int 2 during a collision, if it did, the new key value will overwirte the previous one. We can free the old keyvalue pointer to update with new keyvalue pointer.
