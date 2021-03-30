# ALGORITHMS AND PRINCIPLES OF COMPUTER SCIENCE Project 2018/19

Relationship Database Manager
-----------------------------

This project was written as a final test affiliated to the course "Algoritmi e Principi dell' Informatica" (principles of IT and algorithm).

The Project
-----------
The target of this project was to have the student design a specific case algorithm able to handle a raw database. All the 
input data were to be saved under certain rules into a data structure, and after a request command the software had to return the current state
of memory.

Rules
----------------------
This was an individual project, all the design and coding had to be personal. Plagiarism or cheating was punished with a failed project.
To be able to pass the project, the software had to perform under some space-time boundaries.
All the evaluation was handled by a central server.

Specifications
--------------

The idea behind the project is that the software would recive entities and relations between entities as input, store them, and return the relations' status when asked, as a sort of social network.


#### Environment:
All the code was written in `C11`, using only `stdlib` libraries.

#### Input Characters:
The input characters are restricted from 127 `ASCII`  to only 64 allowed:
- '-' Symbol;
- 0...9 Numbers;
- A...Z Letters;
- "_" Symbol;
- a...z Letters;

All the input entities and relations have double quotes (" "). 

#### Input Commands:

The software execution is regulated by input commands. The input can work either on the *Entities* or onto the *Relations*, and those 
memory-modifying commands can add and delete such data.

Commands | Entities | Relations
--------- | --------|-----
Addition | addent | addrel
Deletion | delent | delrel

##### AddEnt  `addent <id_ent>`

Adds an entity inside the entities databse, but only if the `id_ent` isn't already monitored. If the entity is already monitored, does nothing.

##### AddRel  `addrel <id_ent1> <id_ent2> <id_rel>`

Adds a relationship `id_rel` between `id_ent1` and `id_ent2`, where the first is the source of the relation and the second is the recipient.
If the relationship with given id is already monitored, stores only the data concerning the relations between those entities under given relation.
If the relation doesn't exists in memory, adds it and then stores the data.
If either the relation already binds those entities, or one or both entities aren't monitored in the entities database, the command does nothing.
    
  
##### DelEnt `delent <id_ent>`

Looks for `id_ent` inside the entities database and if it finds it, removes all of its relations in which is involved, and then deletes it from the entities database. If the software doesn't find it, does nothing.


##### DelRel `delrel <id_ent1> <id_ent2> <id_rel>`

Deletes the relation's data between the two entities. If there is no said relation between the entities, does nothing.


#### Execution Commands

##### Report `report`

This is the output command. When parsed, this command had to print out the current state of memory concerning relations.
The output contains all the relations that have at least one couple saved under them. The format of te output is

`<id_rel1> <id_ent1>...<id_entN> <rec_N>;  <id_rel2> <id_ent1>...<id_entN> <rec_N>;...`

The relations are printed in lexicographical order using the ASCII values to evaluate upper and lower cases.
Each relation is followed by one or more of the recipients involved. The entities are chosen amongst those who have the most number of sources for the given relation, parity allowed.
After all the max recipients, the software prints the number that define the max recipients. 

If there are no relations in memory, or every relation has no entities bindings, this command returns `none`



##### End  `end`

Terminates the execution of the software, stating the end of inputs commands.


I/O Files Examples
-------------------

This is a simple human-readable input file, with its corresponding output

```
addent "Mario_Rossi"
report
addent "Mario_Bianchi"
addrel "Mario_Bianchi" "Mario_Rossi" "follows"
report
addrel "Mario_Rossi" "Mario_Bianchi" "likes"
delrel "Mario_Bianchi" "Mario_Rossi" "follows"
report
delent "Mario_Rossi"
report
end
```
And its corresponding output

```
none
"follows" "Mario_Rossi" 1;
"likes" "Mario_Bianchi" 1;
none
```

Implementation
--------------

My implementation is designed around the *HashTable* data structure. The software starts with the initialization of two separate tables, one for the entities and one for the relations. Those tables have 4097 keys each, and the key calculation algorithm returns the key as:

```
tableKey = inputChar[1] * 64 + inputChar[2];
```

For example if the input is "Test" , `inputChar[1] = T` and `inputChar[2] = e`, the software normalizes the `ASCII` value of the letters to those restricted ([explanation here](https://github.com/TehLowLow/A.P.I.-Project2018-19/blob/e18c8cbdf7a9d3ab4dea62915abab5eb62272e28/main.c#L869)) and finds the value of the key:

```
 tableKey = (84 - 54) * 64 + (101 - 59) = 1962
```

Then the software manages the addition/deletion as described above.

Sources
---------

All the project files except the code are under the `rsc` folder. The folder is divided into 3 different directories.
- The *reference implementation*, the software that the professors wrote as a reference while defining the assignment. 
- The *private test cases* which were used to evaluate the students code and that we didn't have direct access to.
- The *public test cases*, much shorter tests designed to give an idea to the student about the single task, and to test locally the software.

This folder will also contain the italian document that we had as specification.


Notes
-------

The software isn't optimal, and not all the tests cases might work properly.
Some test files are missing due to GitHub's upload size limits, as they are larger than 25MB, but they are not significant, as there are plenty of others for testing, and they are large enough to give the idea.
The software is provided AS IS, it will be no longer mantained and/or perfected, as its an educational project.
For any question, write me an issue or email, and i will try to answer them, but i do not guarantee any anwer.



**Lorenzo Gadolini**, *Politecnico di Milano*.













