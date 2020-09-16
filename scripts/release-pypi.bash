#!/bin/bash
set -e

# Determine OS
if [[ "$OSTYPE" == "darwin"* ]]; then
  MINICONDA_FILENAME=Miniconda3-latest-MacOSX-x86_64.sh
  EXTRA_SETUP_ARGS=""
else
  MINICONDA_FILENAME=Miniconda3-latest-Linux-x86_64.sh
  EXTRA_SETUP_ARGS="--plat-name manylinux1_x86_64"

  export DEBIAN_FRONTEND=noninteractive
  apt update
  apt upgrade -y
  apt install -y g++ make cmake curl
fi

# Download and init miniconda
curl -L -o $MINICONDA_FILENAME \
    "https://repo.continuum.io/miniconda/$MINICONDA_FILENAME"
bash ${MINICONDA_FILENAME} -b -f -p $HOME/miniconda3
export PATH=$HOME/miniconda3/bin:$PATH
eval "$(conda shell.bash hook)"

# Go to root directory of package
cd $(dirname $(realpath $0))/..

# Download and build static deps
make download-build-static-deps

# Build packages
for VERSION in 3.6 3.7 3.8; do
    # Create and activate environment
    conda create -y -n py${VERSION} python=${VERSION}
    conda activate py${VERSION}

    # Build and package
    pip install --no-cache-dir setuptools wheel
    python setup.py build_ext bdist_wheel ${EXTRA_SETUP_ARGS}

    # Cleanup
    conda deactivate
    make clean
done

# Upload to PyPI
conda create -y -n twine python=3.8
conda activate twine
python -m pip install twine
python -m twine upload dist/*
