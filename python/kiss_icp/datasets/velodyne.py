# MIT License
#
# Copyright (c) 2022 Ignacio Vizzo, Tiziano Guadagnino, Benedikt Mersch, Cyrill
# Stachniss.
# Copyright (c) 2024 Martin Valgur
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import os

import numpy as np

class VelodynePcapLoader:
    """Velodyne pcap dataloader"""

    def __init__(
        self,
        data_dir: str,
        *_,
        **__,
    ):
        try:
            from velodyne_decoder import read_pcap
        except ImportError as e:
            if e.name == "velodyne_decoder":
                print(f'velodyne_decoder is not installed on your system, run "pip install velodyne_decoder"')
            raise

        assert os.path.isfile(data_dir), "Velodyne pcap dataloader expects an existing PCAP file"

        self.data_dir = data_dir
        # we expect `data_dir` param to be a path to the .pcap file, so rename for clarity
        self._pcap_file = str(data_dir)

        print("Pre-reading Velodyne pcap to count the number of scans ...")
        self._timestamps = np.array([stamp.device for stamp, _ in read_pcap(self._pcap_file)])
        self._timestamps = (self._timestamps - self._timestamps[0]) / (self._timestamps[-1] - self._timestamps[0])
        print(f"Velodyne pcap total scans count:  {len(self._timestamps)}")

        self._scans_iter = read_pcap(self._pcap_file)
        self._next_idx = 0
        self._timestamps = []

    def __getitem__(self, idx):
        assert self._next_idx == idx, (
            "Velodyne pcap dataloader supports only sequential reads. "
            f"Expected idx: {self._next_idx}, but got {idx}"
        )
        stamp, scan = next(self._scans_iter)
        self._next_idx += 1

        # Using device timestamp rather than host's, as it should be more accurate.
        self._timestamps[self._next_idx - 1] = stamp.device

        xyz = scan[:, :3]
        timestamps = scan[:, 4]
        timestamps = (timestamps - timestamps[0]) * (1 / (timestamps[-1] - timestamps[0]))
        return xyz, timestamps

    def get_frames_timestamps(self):
        return self._timestamps

    def __len__(self):
        return len(self._timestamps)

    @staticmethod
    def is_velodyne_pcap(pcap_path, max_packets=100):
        try:
            from velodyne_decoder.util import iter_pcap, parse_packet
            from velodyne_decoder import PACKET_SIZE
        except ImportError as e:
            if e.name == "velodyne_decoder":
                print("[WARNING] Could not load velodyne_decoder to check if pcap is a Velodyne dataset")
                return False
            raise
        for i, (stamp, packet) in enumerate(iter_pcap(pcap_path)):
            if i > max_packets:
                break
            if len(packet) == PACKET_SIZE:
                # Check if the return mode info is valid as a sanity check
                dual_return_mode = parse_packet(packet)[3]
                if dual_return_mode in [0x37, 0x38, 0x39]:
                    return True
        return False
