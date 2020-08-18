# How to Load Windows Icon from Memory

When a Win32 application specifies an icon, it needs to provide an `HICON` handle to the system.
Typically, `HICON` is loaded using Win32 API, either from a file on the disk or from the pre-compiles resources. However, Win32 API doesn't have a straightforward way to load an icon from meory, which can be useful when the icon is generated dynamically. In this case, compiling it as a resource isn't an option and loading from disk is too inefficient.

The following two pages give detailed explanation on the structure of both the ICO file format and the Windows icon resource, as well as how Win32 API read them to load an HICON. It is recommended to give them a read.
* https://en.wikipedia.org/wiki/ICO_(file_format)
* https://docs.microsoft.com/en-us/previous-versions/ms997538(v=msdn.10)

### When a bitmap is in memory

Use [CreateIcon](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createicon) or CreateIconIndirect to create an icon from bitmap data. The arguments are similar to the data fields in an BMP file. The AND bits control the image's transparancy with 1 bit per pixel. The XOR bits are the actual bitmap image without alpha channel.

### When a BMP or PNG image is memory

The API [CreateIconFromResourceEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createiconfromresourceex) has the ability to create an icon from an image. However, Microsoft Docs doesn't specify the format of data it expects.
According to the ICO Wikipedia page, the function can directly read a PNG image. For BMP images, `CreateIconFromResourceEx` expects its header to be removed first and only the bitmap data is provided. So it is actually similar to to how `CreateIcon` works.

### When a ICO image is in memory

Win32 API doesn't have direct support for loading icons from an ICO image. An ICO image can contain multiple icons of different sizes, so the correct icon needs to be first determined. One thing close is [LookupIconIdFromDirectoryEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-lookupiconidfromdirectoryex) which looks up an icon of specific size in an icon resource. This function can also work on ICO image with some modification to the image structure. Both pages in recommended reading discuss about the difference between an ICO image and a Windows icon resource. In a icon resource, the last field of each header entry is a 16 bits id instead of a 32 bits offset. By modifying that an ICO image can be converted into an icon resource and make use of `LookupIconIdFromDirectoryEx`.

However, an easier is probably to look up the correct image by youself by navigating the ICO image structure. An ICO file internally stores icons with BMP or PNG format. Once the size, offset and format of the image is clear, `CreateIconFromResourceEx` can be used to load the icon as described above.