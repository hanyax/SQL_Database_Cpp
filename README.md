# ECE141-SP20-Assignment-9
### Due Friday - June 5, 2020 - 11:15pm 

A relational database consists of multiple related tables linking together using common columns which are known as foreign key columns. Because of this, data in each individual table is often incomplete from the business perspective.  A join is a method of linking data between two or more tables based on values of the common column between the tables (primary and foreign keys).

## Overview -- Table Joins

As we discussed in lecture, there are many types of table joins: `inner, full, left, right, self, cross`. In this assignment, we'll focus on two them:  `left` and `right`.  Please refer to the recent lecture on joins for an explanation.

<hr>

## Key Types in This Assignment 

You'll discover that the following classes are significant in this assignment:

### The `Join` class

The `Join` class is a small "helper" for the `SelectStatement` class. Use this to handle the portion of a SELECT query that includes a JOIN clause, like situations like this:

```
SELECT users.first_name, users.last_name, order_number 
FROM users
LEFT JOIN orders ON users.id=orders.user_id
```

The `JOIN` class holds the data you gather if your SELECT statement includes a JOIN clause.  Mainly, this includes the table(s) you are joining together, the JOIN type, and the fields from each table in the join (the 'ON' expression).

### The `gJoins` array

Below, we've provided a simple array you should add to your existing `helpers.hpp` file, that defines a set of `Keywords` that specify the join types. We use this array during parsing of the `JOIN` clause.  It's up to you to make sure each of these keywords is in your keywords list. 

```
  static ECE141::Keywords gJoinTypes[]={
    ECE141::Keywords::cross_kw, ECE141::Keywords::full_kw, ECE141::Keywords::inner_kw,
    ECE141::Keywords::left_kw,  ECE141::Keywords::right_kw
  };
```

### The `in_array` function

This `in_array` template function let's us do a quick query to determine if a keyword is included in an array of keywors (such as `gJoinTypes` above). Make sure this function is included in your `Helpers.hpp` file.  We use this function in the `parseJOIN` method. It's also a really cool idiom -- because the function can automatically determine the size of the array you pass to the function!

```
  template<typename T, size_t aSize>
  bool in_array(T (&anArray)[aSize], const T &aValue) {
    return std::find(std::begin(anArray), std::end(anArray), aValue);
  };
  
  //example usage...
  if(in_array<Keywords>(gJoinTypes, theToken.keyword)) {
    //do something...
  }
```

### The `parseJoin` method

We provide an outilne of a new method, `paresJoin` that you can include in your code for use in parsing `SELECT` statements.  This method will be called by your `SelectStatement::parse()` method, if/when it encounters a JOIN type. This indicates that your `SELECT` statement has a `JOIN` clause, and that this method should be called to handle it.  

> NOTE: You may have to adapt this method to work in your code. However, it offers a reasonable starting place.

```
  //jointype JOIN tablename ON table1.field=table2.field
  StatusResult SelectStatement::parseJoin(Tokenizer &aTokenizer) {
    Token &theToken = aTokenizer.current();
    StatusResult theResult{joinTypeExpected}; //add joinTypeExpected to your errors file if missing...

    Keywords theJoinType{Keywords::join_kw}; //could just be a plain join
    if(in_array<Keywords>(gJoinTypes, theToken.keyword)) {
      theJoinType=theToken.keyword;
      aTokenizer.next(1); //yank the 'join-type' token (e.g. left, right)
      if(aTokenizer.skipIf(Keywords::join_kw)) {
        std::string theTable;
        if((theResult=parseTableName(aTokenizer, theTable))) {
          Join theJoin(theTable, theJoinType, std::string(""),std::string(""));
          theResult.code=keywordExpected; //on...
          if(aTokenizer.skipIf(Keywords::on_kw)) { //LHS field = RHS field
            TableField LHS("");
            if((theResult=parseTableField(aTokenizer, theJoin.lhs))) {
              if(aTokenizer.skipIf(Operators::equal_op)) {
                if((theResult=parseTableField(aTokenizer, theJoin.rhs))) {
                  joins.push_back(theJoin);
                }
              }
            }
          }
        }
      }
    }
    return theResult;
  }
```

> NOTE: This method includes calls to the method, `parseTableField` -- a method you need to provide yourself. This method tries to parse the name of a table for your statement. You may already have a similar method, and can subtitute that instead.

<hr>

## Integrating files from assignment #8 with this assignment

To do this assignment, you will copy all your files from assignment-#8 project folder into this folder. You don't have to worry about overwriting anything, as there are no files in this folder that were also in previous assignments.

### Integrating the `JOIN` class

Add the `JOIN` class to your `SelectStatement.hpp` file (or whatever you called it. 

```
struct Join  {
    Join(const std::string &aTable, Keywords aType, const std::string &aLHS, const std::string &aRHS)
      : table(aTable), joinType(aType), onLeft(aLHS), onRight(aRHS) {}
        
    Keywords    joinType;
    std::string table;
    TableField  onLeft;
    TableField  onRight;
  };  
```

Next, in your `SelectStatement` class, add a new data member:
```
class SelectStatement {
  //your existing stuff here...
  
protected:
  //all your existing members...
  std::vector<Join> joins;
};
```

<hr>

## Implementing  `LEFT JOIN` 

### For the following examples, we created two tables: books and authors:

```
> select * from Authors;
+----+------------+-----------+
| id | first_name | last_name |
+----+------------+-----------+
|  1 | Stephen    | King      |
|  2 | JK         | Rowling   |
|  3 | Truong     | Nguyen    |
+----+------------+-----------+
3 rows in set (0.00 sec)

> select * from Books;
+----+-------------------------------------------+-----------+
| id | title                                     | author_id |
+----+-------------------------------------------+-----------+
|  1 | Harry Potter and the Sorcerer's Stone     |         2 |
|  2 | Harry Potter and the Philosopher's Stone  |         2 |
|  3 | Harry Potter and the Prisoner of Azkaban  |         2 |
|  4 | Harry Potter and the Chamber of Secrets   |         2 |
|  5 | Harry Potter and the Goblet of Fire       |         2 |
|  6 | Harry Potter and the Order of the Phoenix |         2 |
|  7 | Harry Potter and the Half-Blood Prince    |         2 |
|  8 | Carrie                                    |         1 |
|  9 | The Dark Tower                            |         1 |
| 10 | The Green Mile                            |         1 |
| 11 | Wavelets and Filter Banks                 |         0 |
+----+-------------------------------------------+-----------+
11 rows in set (0.00 sec)
```

#### Implementation Details

As we discussed in lecture, a `LEFT JOIN` selects data starting from the left table. For each row in the left table, the left join compares with every row in the right table. If the values in the two rows cause the join condition evaluates to true (we find matches), the left join creates a new row whose columns contain all columns of the rows in both tables and includes this row in the result set.

If the values in the two rows are not matched, the left join clause still creates a new row whose columns contain specified fields of the row in the left table and NULL for fields specified for the right table.

In other words, the `LEFT JOIN` selects all data from the left table whether there are matching rows exist in the right table or not. In case there is no matching rows from the right table found, NULLs are used for columns of the row from the right table in the final result set.

> NOTE: Remember that it's possible to have authors who haven't had a book published (yet)


#### Based on this data, here are the results from a LEFT join:

```
> select last_name, title from Authors left join Books on Authors.id=Books.author_id;
+-----------+-------------------------------------------+
| last_name | title                                     |
+-----------+-------------------------------------------+
| Rowling   | Harry Potter and the Sorcerer's Stone     |
| Rowling   | Harry Potter and the Philosopher's Stone  |
| Rowling   | Harry Potter and the Prisoner of Azkaban  |
| Rowling   | Harry Potter and the Chamber of Secrets   |
| Rowling   | Harry Potter and the Goblet of Fire       |
| Rowling   | Harry Potter and the Order of the Phoenix |
| Rowling   | Harry Potter and the Half-Blood Prince    |
| King      | Carrie                                    |
| King      | The Dark Tower                            |
| King      | The Green Mile                            |
| Nguyen    | NULL                                      |
+-----------+-------------------------------------------+
11 rows in set (0.00 sec)
```
<hr>

## Implementing  `RIGHT JOIN` 

#### Based on the data provided (above), here are the results from a RIGHT JOIN:

```
> select last_name, title from Authors right join Books on Authors.id=Books.author_id;
+-----------+-------------------------------------------+
| last_name | title                                     |
+-----------+-------------------------------------------+
| King      | Carrie                                    |
| King      | The Dark Tower                            |
| King      | The Green Mile                            |
| Rowling   | Harry Potter and the Sorcerer's Stone     |
| Rowling   | Harry Potter and the Philosopher's Stone  |
| Rowling   | Harry Potter and the Prisoner of Azkaban  |
| Rowling   | Harry Potter and the Chamber of Secrets   |
| Rowling   | Harry Potter and the Goblet of Fire       |
| Rowling   | Harry Potter and the Order of the Phoenix |
| Rowling   | Harry Potter and the Half-Blood Prince    |
| NULL      | Wavelets and Filter Banks                 |
+-----------+-------------------------------------------+
11 rows in set (0.00 sec)
```

As we discussed in class,  `RIGHT JOIN` clause is similar to the `LEFT JOIN` clause except that the treatment of tables is reversed. The `RIGHT JOIN` starts selecting data from the right table instead of the left table. We select all rows from the right table and match rows in the left table. If a row from the right table does not have matching rows from the left table, fields specified from of the left table will have NULL in the final result set.

## Testing This Assignment

### LEFT JOIN vs RIGHT JOIN 

In this assignment read.me, we provided two tables (`Authors`, `Books`) with sample data. You can test your code (locally) using a script file that creates these two tables, inserts records, and then performs LEFT and RIGHT joins to compare you results with those given (above).  We've included a test scripts (available in testfiles/ folder) with sample commands for this purpose. You're free to modify this script for testing in any way you like.  

Assuming you're using test tables described in this readme, you're going to want to make sure your DB can execute the following commands correctly (based on examples above):

```
select last_name, title from Authors LEFT JOIN Books on Authors.id=Books.author_id;
select last_name, title from Authors RIGHT JOIN Books on Authors.id=Books.author_id;
```
The autograder checks the basic scripts which we have provided for you and there are no points for these scripts. This check is to make sure your code compiles with the autograder environment. 

> NOTE: Your score for this assignment will be based on a 3rd script (which will be released after the deadline) -- similar but not exactly the same as those you'll use for testing. As always you can use the auto-grader to help get your code to perform as expected.


## Turning in your work 

Make sure your code compiles, and meets the requirements given above. Also make sure you updated your students.json file with the name of each contributor.  Submissions that don't include a completed `students.json` file will receive 0.

Submit your work by checking it into git by <b>Friday - June 05, 2020 - 11:15pm </b>. Good luck! 
