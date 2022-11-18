.PHONY: refresh build install build_dist json release lint test clean

refresh: clean build install lint

build:
	python setup.py build

install:
	python setup.py install

build_dist:
	make clean
	python setup.py sdist bdist_wheel
	pip install dist/*.whl
	make test

json:
	python example/generate_examples.py

release:
	python -m twine upload dist/*

test:
	python -m unittest

clean:
	rm -rf __pycache__
	rm -rf tests/__pycache__
	rm -rf src/pycodetrace/__pycache__
	rm -rf build
	rm -rf dist
	rm -rf pycodetrace.egg-info
	rm -rf src/pycodetrace.egg-info
	pip uninstall -y pycodetrace
