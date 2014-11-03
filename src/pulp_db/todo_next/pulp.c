
// Can have hard coded context.

// Building a database

struct pulp_context p;

pulp__open(p, "db_name". "w")
unsigned long long msg_i = pulp__append(p, data, n);
pulp__index(p, msg_i, "field_name (null term)", field_value, n)  // field_value is a void *
pulp__index(p, msg_i, "field_name (null term)", field_value, n)
pulp__index(p, msg_i, "field_name (null term)", field_value, n)
pulp__optimize(p)  // Could be combined with below, but this has the high runtime requirement maybe best to symbolise this.
pulp__close(p)




// How does pulp__index work?
// First look up field_name
// A field_name coresponds to an rtrie and a dpref.
// Each field_value corresponds to an int.
// each in coreresponds to a element in an array of dpref buffers

// On pulp__optimize
// for each field rtrie convert it into ttrie
//     iterate each element of the ttrie.
//         convert the dpref to something like cpref. depending on one to one, percentage of population etc.
//         Convert the index to a initial offset kinda value.
//     persist the ttrie.
// need to save any global meta.


// Reading
// Find all key_fields.
// Load the ttries into memory
// Answer the query.



////////////////////////////

/*


// Hard coded context in c.

pulp_w = pulp__open_w()
pulp_r.append()
pulp_w.index()
pulp_w.close()


pulp = pulp__open_r()
pulp_r.index()
pulp_r.close()


// How do we do this?
// Pre generated structs.  Each struct coresponds to an index.
// When the struct is return we have malloced the context into the struct.


256 contexts per 

*/

