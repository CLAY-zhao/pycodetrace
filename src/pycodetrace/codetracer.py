import inspect
from typing import Union, Any

import pycodetrace.codetrace as codetrace


class CodeTracer(object):
    
    def __init__(
        self,
        max_stack_depth: int = 10,
        output_file: str = None
    ) -> None:
        self.initialized = False
        self.enable = False
        self.tracer_list = []
        self.tracerfunc_stack = []
        self.__max_stack_depth = max_stack_depth
        self.output_file = output_file
        self._codetracer = codetrace.CodeTracer()
        self.initialized = True
        
    @property
    def max_stack_depth(self) -> int:
        return self.__max_stack_depth

    @max_stack_depth.setter
    def max_stack_depth(self, max_stack_depth: Union[str, int]) -> None:
        if isinstance(max_stack_depth, str):
            try:
                self.max_stack_depth = int(max_stack_depth)
            except ValueError:
                raise ValueError("Error when trying to convert max_stack_depth {} to integer.".format(max_stack_depth))
        elif isinstance(max_stack_depth, int):
            self.__max_stack_depth = max_stack_depth
        else:
            raise ValueError("Error when trying to convert max_stack_depth {} to integer.".format(max_stack_depth))
        self.config()

    def config(self) -> None:
        if not self.initialized:
            return

        cfg = {
            "max_stack_depth": self.max_stack_depth,
            "output_file": self.output_file
        }

        # self._codetracer.config(**cfg)
        
    def watch(self, obj: Any) -> None:
        self._codetracer.watch(obj)
        
    def unwatch(self) -> None:
        pass

    def install(self, func="watch") -> None:
        import builtins
        setattr(builtins, func, self)

    def uninstall(self, func="watch") -> None:
        import builtins
        if hasattr(builtins, func):
            delattr(builtins, func)