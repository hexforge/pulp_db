# http://docs.python.org/3/library/dbm.html
"""
#  master.data
#  master.index
#  key.index
#  key.data

"""
import sys
import os
import shutil

try:
    from contextlib import ExitStack
except ImportError:
    from xstack_compat import CompatExitStack as ExitStack


import functools

from .msg import Msg
from .mds.mds_pyapi import MasterTable
from .kds.kds_pyapi import KeyTable
from .query import AND, FieldQuery, VirtualQuery, StreamQuery

#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
#
#
# TODO: This should have two modes.  Normal open. 
# ALso should have markup mode, where it takes a file, and just stores offsets and sizes.
# File path should be given at query time.
#
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

def open_pulp_datastore(db_name, mode, msg_dumper=None, msg_loader=None, idx_dumpers=None, idx_loaders=None):
    """
    The tables PulpWritter and PulpReader are separate 
    illustrate that pulp data stores are immutable once created.
    """
    if mode == 'r':
        return PulpReader(db_name, msg_dumper, msg_loader, idx_dumpers, idx_loaders)
    elif mode == 'w':
        return PulpWritter(db_name, msg_dumper, idx_dumpers)
    else:
        raise NotImplementedError("Invalid mode")

class PulpWritter(object):
    """Use this to create a pulp db."""
    def __init__(self, db_name, msg_dumper=None, idx_dumpers=None):
        self.dir_path = os.path.abspath(db_name)
        self.keys_path = os.path.join(self.dir_path, 'keys')
        if os.path.isdir(self.dir_path):
            shutil.rmtree(self.dir_path)
        os.makedirs(self.dir_path)
        os.makedirs(self.keys_path)

        self.master_table = None
        self.key_tables = {}
        self.table_stack = None
        self.msg_dumper = msg_dumper
        self.idx_dumpers = idx_dumpers
    
    def __enter__(self):
        self.table_stack = ExitStack().__enter__()
        table = MasterTable(self.dir_path, 'w', dumper=self.msg_dumper)
        self.add_table(None, table)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self.table_stack is not None:
            self.table_stack.close()

    def add_table(self, key, table):
        if key == None:
            self.master_table = table
        else:
            self.key_tables[key] = table
        self.table_stack.enter_context(table)

    def append(self, data, index_map):
        msg_num = self.master_table.append(data)
        for key, value in index_map.items():
            table = self.key_tables.get(key, None)
            if not table:
                if self.idx_dumpers is not None:
                    dumper = self.idx_dumpers.get(key, None)
                else:
                    dumper = None
                table = KeyTable(self.keys_path, key, 'w', dumper=dumper)
                self.add_table(key, table)

            if isinstance(value, (tuple, list, set)):
                for v in value:
                    table.append(v, msg_num)
            else:
                table.append(value, msg_num)


class PulpReader(object):
    """
    db.idx 
      db.idx(stuff) == KeyQuery or vQuery
    
    db.stream = StreamQuery
        db.stream(stuff) == StreamQuery or vQuery
    
    db.vQuery


    # All three main query objects support the same api.

    KeyQuery
    StreamQuery
    vQuery

    Thus they can be merged.

    """
    def __init__(self, db_name, msg_dumper=None, msg_loader=None, idx_dumpers=None, idx_loaders=None):
        self.dir_path = os.path.abspath(db_name)
        self.keys_path = os.path.join(self.dir_path, 'keys')
        if not all(os.path.exists(p) for p in [self.dir_path, self.keys_path]):
            print("Missing directory: one of {}".format([self.dir_path, 
                                                         self.keys_path]))
        self.table_stack = None
        self.master_table = None
        self.key_tables = {}
        self.msg_dumper = msg_dumper
        self.msg_loader = msg_loader
        self.idx_dumpers = idx_dumpers
        self.idx_loaders = idx_loaders
    
    @property
    def stream(self):
        return StreamQuery(self.master_table)

    @property
    def idx(self):
        return FieldQueryDispatch(self.master_table, self.key_tables)

    def __enter__(self):
        self.table_stack = ExitStack().__enter__()
        table = MasterTable(self.dir_path, 'r', dumper=self.msg_dumper ,loader=self.msg_loader)
        self._add_table(None, table)
        
        keys = [os.path.splitext(f)[0] for f in os.listdir(self.keys_path) if f.endswith('.meta')] ## More needed here.  More than just .meta files required.
        for key in keys:
            if self.idx_dumpers is not None:
                dumper = self.idx_dumpers.get(key, None)
            else:
                dumper = None
            if self.idx_loaders is not None:
                loader = self.idx_loaders.get(key, None)
            else:
                loader = None
            table = KeyTable(self.keys_path, key, 'r', dumper=dumper, loader=loader)
            self._add_table(key, table)
        return self

    def _add_table(self, key, table):
        if key == None:
            self.master_table = table
        else:
            self.key_tables[key] = table
        self.table_stack.enter_context(table)

    def __exit__(self, exc_type, exc_value, traceback):
        if self.table_stack is not None:
            self.table_stack.close()

    def __len__(self):
        """Dispatches to stream"""
        return len(self.stream)

    def __iter__(self):
        return iter(self.stream)

    def __getitem__(self, index):
        """Dispatches to stream"""
        if isinstance(index, int):
            return self.stream[index]
        elif isinstance(index, slice):
            return self.stream[index]
        else:
            raise NotImplementedError("index")

class FieldQueryDispatch(object):
    def __init__(self, master_table, key_tables):
        self.__key_tables = key_tables
        self.__master_table__ = master_table

    def __getitem__(self, key):
        return FieldQuery(self.__key_tables[key], self.__master_table__)
    
    def __call__(self, **kwargs):
        all_qs = []
        for key, thing in kwargs.items():
            if not callable(thing):
                if isinstance(thing, bytes):
                    func = lambda x: x == thing
                else:
                    raise NotImplementedError("Worktodo here")
                print(func)
            else:
                func = thing

            fld_call_query = FieldQuery(self.__key_tables[key], 
                                        master_table=self.__master_table__
                                        ).__call__(func)
            if fld_call_query is None:
                print("Error: Null field query, no such fields?", key)
                sys.exit(1)
            all_qs.append(fld_call_query)

        addq = functools.partial(VirtualQuery, 
                                 operator=AND, 
                                 master_table=self.__master_table__)

        all_query = functools.reduce(addq, all_qs)
        return all_query

    def keys(self):
        return self.__key_tables.keys()

    def fields(self):
        return self.keys()
