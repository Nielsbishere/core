# Model compression (oiRMc)

For an example, we will take a uncompressed oiRM of 3 012 848 bytes; the anvil model included in the example.

## VBO compression

By analyzing the data, you can see that there isn't that much variation for position, uv and normal data. The anvil has the following variation in pos, uv and nrm channels:

| Type | Bytes | Options | Channel type |
| ---- | ----- | ------- | ------------ |
| Pos  | 2 544 | 636     | f32          |
| Uv   | 3 868 | 967     | f32          |
| Nrm  | 7 824 | 1 956   | f32          |

Whenever we reference a position, we don't require 3 * 4 = 12 bytes, we just require the values in a keyset; and then we need 30 bits to reference to it. We have 54 656 positions, yet there are only 636 options per channel. This means we have to store 636 * 4 = 2 544 bytes and 30 * 54 656 / 8 = 204 960 bytes instead of 54 656 * 12 = 655 872 bytes.

This requires a buffer with the keys and a bitset with the indices to the keys.

This can be taken even further by checking how many different combinations are used per attribute. In our example, we have the following combinations:

| Type | Bytes  | Options | Bit depth | Channels |
| ---- | ------ | ------- | --------- | -------- |
| Pos  | 49 540 | 13 210  | 10        | 3        |
| Uv   | 35 728 | 14 291  | 10        | 2        |
| Nrm  | 26 892 | 6 519   | 11        | 3        |

Now if we reference a position; it takes 14 bits instead of 96. This gives us the total of `2 544 + 49 540 + 14 * 54 656 / 8 = 2 544 + 49 540 + 95 648 = 147 732` instead of `204 960` or `655 872` for position.

| Type | No comp | Per channel | Per attribute |
| ---- | ------- | ----------- | ------------- |
| Pos  | 655 872 | 204 960     | 147 732       |
| Uv   | 437 248 | 140 508     | 135 244       |
| Nrm  | 655 872 | 233 280     | 123 532       |
| Ind  | 327 936 | 163 968     | -             |

The index (ind) buffer is used so vertices aren't duplicated. Instead of using a `u32[]`, we use a bitset with n options, where n is the number of vertices. Since we have 54 656 vertices, you can use a u16. Normally the index buffer type is used to do this (so <=256 vertices uses u8, <=65536 uses u16 and otherwise it uses u32), but our technique becomes very efficient when you hit 65536 vertices, because then instead of 32-bits, you are using 17. 

After all compression, the file turns out to be 830 845 bytes (it also has things like headers and things that take up more info too.), instead of 3 012 848 bytes. 28% of the original size. When compressed, this is 465 929 bytes; 15% of the original size.

## IBO compression

Most of the model file is indices to indices to values; there is almost no raw data left (about 11 / 830 K). This means that if the indices are optimized, there could be a significant improvement in storage; maybe 500K (non-zipped) is accomplishable. This is only possible if the indices follow some sort of pattern, which for Index buffer is pretty easy to grasp

```
2, 1, 0, (3x u16)
3, 2, 0,
6, 5, 4,
7, 6, 4
```

4 triangles are actually 2 quads. If we find a way to specify that in less than 3 * 4 * 2 = 24 bytes, we can save a lot of storage.

```
triangle increase index right to left 0; n + 2, n + 1, n
triangle increase index right to left 0 step 2 once; n + 3, n + 2, n
triangle increase index right to left 4; n + 2, n + 1, n
triangle increase index right to left 4 step 2 once; n + 3, n + 2, n
```

If we go through the triangles and rank them in 4 options; 00 (3x ind), 01 (1 ind; inc left to right), 10 (1 ind; inc right to left) and 11 (1 ind; inc right to left and step of 2 after 1st ind), we get the following results (27 328 indices):

|        | 00   | 01   | 10     | 11     |
| ------ | ---- | ---- | ------ | ------ |
| Number | 0    | 0    | 13 664 | 13 664 |
| Bits   | 0    | 0    | 27 328 | 27 328 |

This means that we require 2 bits per triangle at least, and in our case; 50 max.

We would go from 163 968 bytes to 13 664 * 2 * (2 + 16) / 8 = 61 488 bytes (37.5% of original).

In modern modeling software, artists model by quads (not triangles), which means this technique is very well made for that. The only drawback is that other triangles that don't follow that logic will still use the regular indices; so still take a max of 50 bits per triangle. 

This technique would reduce the 830 845 byte anvil oiRMc file to 728 365 bytes (24.1% of original and 88% of compressed). Which means that indices take up 8.4% of the total file, instead of 22.5%. 

With this in mind; we know that CW isn't produced normally by the Fbx/Obj converter; it automatically uses CCW. This means that we could use 01 as a quad; saving 50% space (for index buffer). Keep in mind that we can only apply this technique to triangle mode oiRMs.

With this technique; using 01 as quad; we see the following:

|        | 00   | 01     | 10   | 11   |
| ------ | ---- | ------ | ---- | ---- |
| Number | 0    | 13 664 | 0    | 0    |
| Bits   | 0    | 27 328 | 0    | 0    |

Meaning we go from 163 968 bytes to `13 664 * (2 + 16) / 8 = 30 744 bytes` (18.75% of original). While the original buffer in memory would be 327 936 bytes (9.375%).