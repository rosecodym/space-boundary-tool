// Copyright (c) 1997-2000  
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved. 
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of the License,
// or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Hash_map/include/CGAL/Tools/chained_map.h $
// $Id: chained_map.h 67093 2012-01-13 11:22:39Z lrineau $
// 
//
// Author(s)     : Courtesy of LEDA
#ifndef CGAL_CHAINED_MAP_H
#define CGAL_CHAINED_MAP_H

namespace CGAL {

namespace internal {

template <typename T> class chained_map;
template <typename T> class chained_map_elem;

template <typename T>
class chained_map_elem 
{
  friend class chained_map<T>;
  std::size_t k; T i;
  chained_map_elem<T>*  succ;
};

template <typename T>
class chained_map
{
   const std::size_t NULLKEY; 
   const std::size_t NONNULLKEY;

   chained_map_elem<T> STOP;

   chained_map_elem<T>* table;
   chained_map_elem<T>* table_end;
   chained_map_elem<T>* free;
  std::size_t table_size;           
  std::size_t table_size_1;  

   chained_map_elem<T>* old_table;
   chained_map_elem<T>* old_table_end;
   chained_map_elem<T>* old_free;
   std::size_t old_table_size;           
   std::size_t old_table_size_1;  

   std::size_t old_index;

public:
   T& xdef() { return STOP.i; }
   const T& cxdef() const { return STOP.i; }
private:
   void init_inf(T& x)   const { x = STOP.i; }

   
   chained_map_elem<T>*  HASH(std::size_t x)  const
   { return table + (x & table_size_1);  }
   
   void init_table(std::size_t t);
   void rehash();
   void del_old_table();

   inline void insert(std::size_t x, T y);

public:
   typedef chained_map_elem<T>*  chained_map_item;
   typedef chained_map_item item;

   std::size_t index(chained_map_item it) const { return it->k; }
   T&            inf(chained_map_item it) const { return it->i; }

  chained_map(std::size_t n = 1); 
   chained_map(const chained_map<T>& D);
   chained_map& operator=(const chained_map<T>& D);
   

   void clear_entries();
   void clear();
   ~chained_map() { if (old_table) delete[] old_table; delete[] table; }

   T& access(chained_map_item p, std::size_t x);
   T& access(std::size_t x);
   chained_map_item lookup(std::size_t x) const;
   chained_map_item first_item() const;
   chained_map_item next_item(chained_map_item it) const;
   void statistics() const;
};

template <typename T>
inline T& chained_map<T>::access(std::size_t x)
{ chained_map_item p = HASH(x);

  if (old_table) del_old_table();
  if ( p->k == x ) { 
     old_index = x; 
     return p->i;
  }
  else {
    if ( p->k == NULLKEY ) {
      p->k = x;
      init_inf(p->i);  // initializes p->i to xdef
      old_index = x;
      return p->i;
    } else 
      return access(p,x);
  }
}

template <typename T>
void chained_map<T>::init_table(std::size_t t)
{ 
  table_size = t;
  table_size_1 = t-1;
  table = new chained_map_elem<T>[t + t/2];
  free = table + t;
  table_end = table + t + t/2;      

  for (chained_map_item p = table; p < free; p++) 
  { p->succ = &STOP; 
    p->k = NULLKEY;
  }
  table->k = NONNULLKEY;
}


template <typename T>
inline void chained_map<T>::insert(std::size_t x, T y)
{ chained_map_item q = HASH(x);                                    
  if ( q->k == NULLKEY ) {      
    q->k = x;                                                  
    q->i = y; 
  } else { 
    free->k = x;                                                
    free->i = y;                                                
    free->succ = q->succ;                                       
    q->succ = free++; 
  }                                         
}

                                                                            
template <typename T>
void chained_map<T>::rehash()
{ 
  old_table = table;
  old_table_end = table_end;
  old_table_size = table_size;
  old_table_size_1 = table_size_1;
  old_free = free;

  chained_map_item old_table_mid = table + table_size;

  init_table(2*table_size);

  chained_map_item p;

  for(p = old_table + 1; p < old_table_mid; p++)
  { std::size_t x = p->k;
    if ( x != NULLKEY ) // list p is non-empty
    { chained_map_item q = HASH(x);  
      q->k = x;
      q->i = p->i;
    }
  }

  while (p < old_table_end)
  { std::size_t x = p->k;
    insert(x,p->i);
    p++;
  }
}


template <typename T>
void chained_map<T>::del_old_table()
{
  chained_map_item save_table = table;
  chained_map_item save_table_end = table_end;
  chained_map_item save_free = free;
  std::size_t save_table_size = table_size;
  std::size_t save_table_size_1 = table_size_1;

  table = old_table;
  table_end = old_table_end;
  table_size = old_table_size;
  table_size_1 = old_table_size_1;
  free = old_free;

  old_table = 0;

  T p = access(old_index);

  delete[] table;

  table = save_table;
  table_end = save_table_end;
  table_size = save_table_size;
  table_size_1 = save_table_size_1;
  free = save_free;
  access(old_index) = p;
}

template <typename T>
T& chained_map<T>::access(chained_map_item p, std::size_t x)
{
  STOP.k = x;
  chained_map_item q = p->succ; 
  while (q->k != x) q = q->succ;
  if (q != &STOP) 
  { old_index = x;
    return q->i;
  }

  // index x not present, insert it

  if (free == table_end)   // table full: rehash
  { rehash();
    p = HASH(x);
  }

  if (p->k == NULLKEY)
  { p->k = x;
    init_inf(p->i);  // initializes p->i to xdef
    return p->i;
  }

  q = free++;
  q->k = x;
  init_inf(q->i);    // initializes q->i to xdef
  q->succ = p->succ;
  p->succ = q;
  return q->i;
}


template <typename T>
chained_map<T>::chained_map(std::size_t n) : 
  NULLKEY(0), NONNULLKEY(1), old_table(0)
{ 
  if (n < 512)
    init_table(512); 
  else {
    std::size_t ts = 1;
    while (ts < n) ts <<= 1;
    init_table(ts);
  }
}


template <typename T>
chained_map<T>::chained_map(const chained_map<T>& D) : 
  NULLKEY(0), NONNULLKEY(1), old_table(0)
{ 
  init_table(D.table_size);
  STOP.i = D.STOP.i; // xdef

  for(chained_map_item p = D.table + 1; p < D.free; p++) 
  { if (p->k != NULLKEY || p >= D.table + D.table_size)
    { insert(p->k,p->i);
      //D.copy_inf(p->i);  // see chapter Implementation
    }
  }
}

template <typename T>
chained_map<T>& chained_map<T>::operator=(const chained_map<T>& D)
{ 
  clear_entries();
  delete[] table;
  init_table(D.table_size);
  STOP.i = D.STOP.i; // xdef

  for(chained_map_item p = D.table + 1; p < D.free; p++) 
  { if (p->k != NULLKEY || p >= D.table + D.table_size)
    { insert(p->k,p->i);
      //copy_inf(p->i);    // see chapter Implementation
    }
  }
  return *this;
}

template <typename T>
void chained_map<T>::clear_entries() 
{ for(chained_map_item p = table + 1; p < free; p++)
    if (p->k != NULLKEY || p >= table + table_size) 
      p->i = T();  
}

template <typename T>
void chained_map<T>::clear() 
{ clear_entries();
  delete[] table;
  init_table(512); 
}

template <typename T>
typename chained_map<T>::chained_map_item 
chained_map<T>::lookup(std::size_t x) const 
{ chained_map_item p = HASH(x);
  ((std::size_t &)STOP.k) = x;  // cast away const
  while (p->k != x) 
  { p = p->succ; }
  return (p == &STOP) ? 0 : p;
}


template <typename T>
typename chained_map<T>::chained_map_item 
chained_map<T>::first_item() const
{ return next_item(table); }

template <typename T>
typename chained_map<T>::chained_map_item 
chained_map<T>::next_item(chained_map_item it) const 
{ if (it == 0) return 0;
  do it++; while (it < table + table_size && it->k == NULLKEY);
  return (it < free ? it : 0);
}

template <typename T>
void chained_map<T>::statistics() const
{ std::cout << "table_size: " << table_size <<"\n";
  std::size_t n = 0;
  for (chained_map_item p = table + 1; p < table + table_size; p++)
     if (p ->k != NULLKEY) n++;
  std::size_t used_in_overflow = free - (table + table_size );
  n += used_in_overflow;
  std::cout << "number of entries: " << n << "\n";
  std::cout << "fraction of entries in first position: " << 
               ((double) (n - used_in_overflow))/n <<"\n";
  std::cout << "fraction of empty lists: " << 
               ((double) (n - used_in_overflow))/table_size<<"\n";
}

} // namespace internal
} //namespace CGAL

#endif // CGAL_CHAINED_MAP_H
