class Msg(object):
    def __init__(self, msg_id, master_table, msg=None):
        self.master_table = master_table
        self.id = msg_id
        self._msg = msg

    @property
    def msg(self):
        """Looks up the master table, need a shared mView like object"""
        if self._msg is None:
            self._msg = self.master_table[self.id]
        return self._msg

    def __repr__(self):
        return "<Msg({})>".format(self.id)
